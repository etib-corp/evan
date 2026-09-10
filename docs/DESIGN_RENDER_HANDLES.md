# Design: shared render handles with in-place updates

Status: proposal (design only)
Scope: evan + guillaume + xider
Depends on: evan scene-system removal (Renderer now owns the object registry)

## 1. Goals / non-goals

**Goals**

- A render object registered by guillaume stays the *same* evan object across
  frames.
- Transform/tint changes are cheap uniform writes; geometry changes are
  in-place vertex updates.
- No `addMesh`/`addText`/`addModel` calls inside the frame loop.
- No GPU allocation/destruction churn while an object moves or resizes.

**Non-goals (for now)**

- Instancing and shared-geometry batching across identical visuals.
- Immediate-mode rendering.
- Making guillaume depend on evan types.

## 2. Current bottleneck

Every render system (`RectangleRender`, `ImageRender`, `TextRender`,
`GlyphRender`, `ModelRender`) runs once per entity per frame and caches
`pose + content -> engine object id` (e.g. `rectangle_render.cpp`). Pose and
scale are baked into CPU-space vertices, so any motion is a cache miss and the
system calls `add*` (new GPU buffers) and later `removeObject` in `cleanup()`.

Root cause: the evan renderer draws every mesh with `model = identity`
(`sources/Renderer.cpp`) and a hard-coded white push constant, so guillaume has
no way to express "same geometry, different pose" except rebuilding vertices.

## 3. Target model: three roles, one handle

| Role                        | Owner                              | Lifecycle                                        |
| --------------------------- | ---------------------------------- | ------------------------------------------------ |
| Geometry (mesh, UVs, topology) | guillaume builds local-space mesh once | changes rarely (content / radius / font)     |
| Instance / handle           | evan `Renderer::_objects[id]`      | created once via `add*`, mutated in place        |
| Per-object state (mat4 + tint) | evan registry entry             | updated per frame via uniform                    |
| Material                    | evan `RessourceManager`            | changes rarely (asset swap)                      |

## 4. Evan changes

1. Registry entry gains instance data: the value of `Renderer::_objects`
   becomes (or is paired with) a struct holding the `RenderObject`, its model
   matrix, its tint, and a dirty flag.
2. **Per-object uniform.** Add a dynamic uniform buffer (one per frame,
   `MAX_FRAMES_IN_FLIGHT`) with per-object offsets. Each mesh draw binds the
   per-frame view/projection UBO plus the per-object model/tint. The tint
   replaces the hard-coded white push constant; shaders apply `model` in the
   vertex stage (mesh, default and text pipelines).
3. **Multi-frame safety.** Instance data is written *inside* `drawFrame` for the
   current frame, not when `setObjectTransform` is called (guillaume updates
   objects during its update phase, possibly frames before the draw). The
   registry keeps a CPU copy and a dirty flag; only dirty objects are uploaded
   into the current frame's buffer. This composes with the existing
   wait-on-fence logic.
4. **XR note.** Per-object data is view-independent (only view/projection
   differ per eye), so it is written once per frame and shared across views.
5. New `evan::Engine` API (types come from `utility::graphic`, shared by evan
   and guillaume; evan converts to `glm`):
   - `void setObjectTransform(size_t objectID, const PoseF &pose, const ScaleF &scale);`
   - `void setObjectTint(size_t objectID, const Color32Bit &color);`
   - `void updateObjectGeometry(size_t objectID, const utility::graphic::Mesh &mesh);`

   `updateObjectGeometry` routes to `GPUMesh::updateVertices` when vertex counts
   match (guillaume builders keep fixed topology: fixed arc segments, fixed
   quads) and re-creates buffers internally otherwise (rare).
6. Existing `addMesh`/`addText`/`addModel` keep returning the stable object id.

## 5. Guillaume changes

1. **Per-entity render handle.** Each render system keeps
   `entityId -> { objectId, geometryKey }` instead of the global
   `pose + content -> id` cache.
2. **Split the per-frame work.**
   - Every frame (cheap): derive pose + scale from `Transform` and call
     `engine->setObjectTransform(handle, ...)`. Push only when the transform
     changed if write-skipping is desired.
   - On geometry change only: rebuild the local-space CPU mesh when
     `geometryKey` changes. The key covers size, borders/radius,
     color-as-vertex, material/texture path, text content/font, glyph — but
     **never pose or scale**. Then call `engine->updateObjectGeometry(handle, mesh)`.
   - First sight / destruction: call `add*` once when the visual first appears
     and `removeObject` once when the entity or visual is destroyed.
3. **Local-space builders.** Stop baking pose/scale into vertices. Quads are
   centered at the origin; scale/size are applied by the transform matrix.
   Rounded-corner radius still triggers `updateObjectGeometry` (vertex
   positions) but preserves topology when `arcSegments` is fixed.
4. **Abstract `guillaume::Engine`** gains the same three verbs (still
   evan-agnostic). `xider::Engine` forwards them exactly like today's
   `addMesh`. Text and model paths stay specialized but follow the same handle
   pattern (`addText`/`addModel` once per content, transform pushed per frame).

## 6. Frame lifecycle (after)

```
per frame:
  measure / layout phases                 (unchanged)
  render systems:
    for entity:
      setObjectTransform(handle, pose, scale)   # uniform write
      if geometryKey changed: updateObjectGeometry(handle, mesh)
      if new visual:             add*(...)        # once
      if destroyed:              removeObject(handle)
  engine update() -> renderer::drawFrame()
    upload dirty instance data -> record per-object draws -> present
```

No object is created or destroyed while an object merely moves or resizes.

## 7. Consequence of the evan scene removal (lands with it)

guillaume currently registers one evan scene per guillaume scene
(`guillaume::Engine::addScene` is called from `Application::setEngine`,
`application.tpp`, and `xider::Engine::addScene` forwards to the deleted
`evan::Engine::addScene`). With evan scenes removed this mapping must be
redesigned in the same change:

- guillaume owns scene-level object lifecycle: on scene deactivation / exit,
  every handle its systems created is removed (or via a `clearObjects()` verb),
  because there is now a single evan registry.
- `guillaume::Engine::addScene` and the `SceneManager` evan-scene bookkeeping
  are dropped; `xider::Engine` drops its `addScene` override.

## 8. Phased rollout and acceptance

1. **P0 (in progress)** — evan scene-system removal; registry owned by
   `Renderer`.
2. **P1 — evan core:** per-object instance data, dynamic uniform buffer,
   shader/pipeline-layout updates, new verbs, multi-view and in-flight safety.
   - Acceptance: moving a registered object is a small uniform write; no new
     allocations per frame; XR frame path stays green.
3. **P2 — guillaume:** per-entity handles, pose-free geometry keys, local-space
   builders, text/model aligned, scene-exit cleanup, `addScene` machinery
   removed.
   - Acceptance: animating a panel produces zero `add*`/`removeObject` calls
     per frame, only transform writes.
4. **P3 — xider adapter + samples + CI:** forward the new verbs, drop
   `addScene`, update GLFW/OpenXR samples, `ctest`, `clang-format`.

## 9. Risks

- Dynamic-uniform and pipeline-layout rework in evan touches shaders,
  descriptor layouts and the XR multi-view path.
- The guillaume render-system refactor touches all five render systems.
- The guillaume `addScene` / scene-lifecycle redesign is mandatory and
  cross-repo (evan + guillaume + xider must merge in lockstep).
