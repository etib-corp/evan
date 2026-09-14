# evan — Vulkan/OpenXR Performance Audit

## Role in the rendering architecture

`evan` sits at the bottom of the rendering stack and owns everything GPU:

```text
xider (application) ──► guillaume (ECS/application shell) ──► evan (Vulkan/OpenXR)
                                                                   └── utility (shared types)
```

Key classes: `Engine`, `DeviceContext`, `ADeviceBackend` (+ `DesktopBackend`,
`XrDeviceBackend`), `ASwapchainContext` (+ `XrSwapchainContext`,
`DesktopSwapchainContext`), `Renderer`, `Frame`, `Scene`, `RenderObject`,
`GPUMesh`, `GPUMaterial`, `GPUTexture`, `RessourceManager`, `ViewSet`.

Responsibilities owned by `evan`:

- Vulkan instance/device/queue init: `DeviceContext` + backends
  (`DeviceContext.cpp`, `DesktopBackend.cpp`, `XrDeviceBackend.cpp`).
- OpenXR init/session: `XrDeviceBackend` (`XrDeviceBackend.cpp`).
- Command buffer creation/recording: `Frame` (`Frame.cpp`) and
  `Renderer::recordCommandBuffer` (`Renderer.cpp`).
- Submission/sync: `Renderer::drawFrame` (`Renderer.cpp`).
- Scene data: `Engine::addMesh/addModel/addText` →
  `Scene`/`RenderObject`/`GPUMesh`.

The boundary where scaling damage occurs is `guillaume`'s render systems calling
`evan::Engine::addMesh/addModel/addText`: each call builds a full
`RenderObject` → `GPUMesh` (staging buffer + `vkQueueWaitIdle`) and triggers a
`RessourceManager::sync()`.

---

## Executive summary

The dominant `evan` problem is **per-frame CPU work plus cache/queue
synchronization that scales linearly (or worse) with object count, amplified by
an unconditional 60 FPS sleep and a per-eye GPU stall on the OpenXR path**.

1. **Per-mesh logging inside the draw loop** (`Renderer::recordCommandBuffer`
   emits ~5 log lines per mesh per eye per frame) — the single largest
   object-count-scaling CPU cost.
2. **One draw call per mesh, full state re-bind, no culling/batching/instancing.**
3. **The CPU waits for the GPU between the two eyes and again before
   composition** (`Renderer::drawFrame`), fully serializing CPU recording and
   GPU execution in VR.
4. **`Engine::updateDeltaTime()` no longer sleeps to cap the loop at 60 FPS**
   (resolved by P0.2); the loop is paced by the presentation mechanism.
5. **`vkQueueWaitIdle` on every mesh upload / vertex update** drains the
   graphics queue for each new object.

Net: average FPS is CPU-bound and drops ~linearly with object count; VR frame
pacing is structurally unstable.

---

## Frame lifecycle (inside `evan`)

`guillaume::Application::routine` drives the loop; the `evan`-owned parts are:

- `evan::Engine::pollEvents` (`Engine.cpp`) → platform.
- `evan::Engine::update` (`Engine.cpp`):
  - `updateDeltaTime()` — measures delta; applies an opt-in limiter only when
    `setTargetFps()` is configured (`Engine.cpp`).
  - `handleViewportInput()`.
- `evan::Engine::render` → `Renderer::drawFrame` (`Renderer.cpp`).

`Renderer::drawFrame` (OpenXR):

- `preprocessFrame` → `xrWaitFrame` + `xrBeginFrame` + `xrLocateViews`
  (`XrDeviceBackend.cpp`).
- `vkWaitForFences` on the in-flight fence (`Renderer.cpp`).
- Per swapchain: `xrAcquireSwapchainImage` + `xrWaitSwapchainImage` with
  `XR_INFINITE_DURATION` (`XrSwapchainContext.cpp`).
- For each view/eye: `updateUniformBuffer` (memcpy), `resetCommandBuffer`,
  `recordCommandBuffer`, `vkQueueSubmit`.
- **Between eyes: `vkWaitForFences(..., UINT64_MAX)`** (`Renderer.cpp`).
- After eyes: **another `vkWaitForFences`** before release (`Renderer.cpp`).
- `processFrame` → `xrReleaseSwapchainImage`; `postprocessFrame` →
  `xrEndFrame`.

Desktop path is the same minus the OpenXR calls, with
`vkAcquireNextImageKHR`/`vkQueuePresentKHR`.

---

## Ranked findings

| Rank | Bottleneck | Confirmed? | Where |
| ---- | ---------- | ---------- | ----- |
| 1 | Per-mesh logging in the draw loop | CONFIRMED | `Renderer.cpp:675-748` |
| 2 | Per-eye + pre-composition GPU fence waits (no pipelining) | CONFIRMED | `Renderer.cpp:219-245` |
| 3 | 60 FPS sleep in the frame loop | RESOLVED | `Engine.cpp`; `DesktopSwapchainImage.cpp` |
| 4 | One draw call + full state re-bind per mesh; no culling/batching/instancing | CONFIRMED | `Renderer.cpp:675-748` |
| 5 | `Scene::getMeshes()` reallocates per frame | CONFIRMED | `Scene.cpp:69-80` |
| 6 | `RessourceManager::sync()` per frame + per object-add (map copies + prefix scans) | CONFIRMED | `RessourceManager.cpp:77-174`; `Engine.cpp:139,166,190,222,238` |
| 7 | `vkQueueWaitIdle` on every mesh upload / vertex update | CONFIRMED | `ADeviceBackend.cpp:214-241`; `GPUMesh.cpp:80,161` |
| 8 | 4× MSAA + blending on all geometry, 2× full passes (no multiview) | LIKELY (GPU) | `DeviceContext.cpp:178-181`; `Renderer.cpp:442-461` |
| 9 | `GPUMaterial::getDescriptorSets()` returns vector by value | RESOLVED | `GPUMaterial.hpp:150`; `Renderer.cpp:876,1035` |
| 10 | No `VkPipelineCache` (pipelines created with `VK_NULL_HANDLE`) | CONFIRMED | `Renderer.cpp:538` |

---

## Object scaling analysis

Let N = meshes, M = materials, V = views per frame (1 desktop, 2 OpenXR),
E = resource-provider element count.

Per frame, in `Renderer::recordCommandBuffer`:

- `scene.getMeshes()` (`Scene.cpp:69`) builds a fresh
  `std::vector<shared_ptr<GPUMesh>>`, copying every mesh's `shared_ptr` —
  **O(N) with heap reallocation** (no `reserve`).
- Per mesh: `_ressourceManager->getMaterial` (hash lookup), two `_pipelines`
  map lookups, `vkCmdBindPipeline`, `vkCmdBindVertexBuffers`,
  `vkCmdBindIndexBuffer`, optional descriptor bind, `vkCmdPushConstants`,
  `vkCmdDrawIndexed`, and **~5 log statements**.
- Per frame: **~6 Vulkan calls + 5 formatted/flushed log records per mesh**,
  times V.

Per frame, `_ressourceManager->sync()` (`Renderer.cpp`, once per view):

- Copies `_shaders`, `_materials`, `_textures` maps **by value**.
- For each material calls `_ressourceProvider->getShaderID(name)`, which scans
  `_elementsIDs` with `starts_with` — **O(M·E)**.

Renderer scaling:

- Draw-call scaling: **O(N)** (no culling/batching/instancing).
- CPU recording scaling: **O(N · V)** with a very large constant (logging +
  state calls).
- Resource sync scaling: **O(V · M · E)** per frame, independent of N but paid
  every frame.
- Object *addition* cost: `addMesh/addModel/addText` runs `sync()` and
  constructs a `GPUMesh` with staging buffer + `copyBuffer` →
  `vkQueueWaitIdle`. Adding N objects at once is **O(N·(M·E + GPU sync))**.

---

## CPU bottlenecks

- **Logging in hot loops (CONFIRMED, highest impact).** ~5 formatted+flushed
  log lines per mesh per eye. Call sites: `Renderer.cpp:676,696,705,713,744`.
  (The logger itself is `utility` — see the utility audit.)
- **Per-frame allocation in `Scene::getMeshes()`** (`Scene.cpp:69`).
  `GPUMaterial::getDescriptorSets()` previously returned a `std::vector` by
  value; it now returns a const reference (`GPUMaterial.hpp:150`).
- **`RessourceManager::sync()` map copies + `getShaderID` prefix scan**
  (`RessourceManager.cpp:89-95,121`) every frame and on every object add.
- **`std::map`/`unordered_map` lookups per mesh** for material and pipeline
  (`Renderer.cpp:679,689,700`). `_pipelines` is `std::map` (tree), so each
  lookup is O(log P).
- **`Engine::updateDeltaTime()` no longer sleeps by default** — the loop is
  paced by vsync (desktop FIFO) or `xrWaitFrame` (OpenXR); an opt-in
  `setTargetFps()` limiter is available.

Not significant: `getPipelineLayer()` returning `std::string` by value
(`RenderObject.hpp:107`) is not on a hot path.

---

## GPU bottlenecks

Static analysis cannot prove GPU bottlenecks; these are hypotheses to confirm
with timestamps/RenderDoc:

- **No culling** (frustum/distance/occlusion). Every mesh is submitted.
- **No instancing/batching** — identical meshes are separate draws.
- **No multiview** — stereo is two full passes (`Renderer.cpp`), doubling
  vertex/fragment work and command recording.
- **4× MSAA** by default (`DeviceContext.cpp:178-181`) with
  `resolveToSwapchain` (`XrSwapchainContext.cpp`) — 4× color/depth bandwidth.
- **Blending always on** for every pipeline (`Renderer.cpp:453-461`), even for
  opaque geometry, and no front-to-back sorting, so overdraw/ROP cost is
  unbounded.
- Uniform buffer read per vertex with an identity `model` matrix
  (`Renderer.cpp:597`), so world transforms are baked on the CPU — the GPU does
  no per-instance transform, but the CPU must regenerate geometry on pose
  change.

Because draw-call count is the only thing that scales with N on the GPU side,
the GPU is likely **not** the first bottleneck until N is large; the
CPU/queue-idle path saturates first.

---

## Vulkan bottlenecks

- **Command buffers:** one primary command buffer per frame, **shared across
  eyes** (`Frame.hpp`, `Frame.cpp`). This forces the inter-eye fence wait in
  `drawFrame` (`Renderer.cpp:219-230`). No multithreaded recording; no
  secondary buffers. Reset strategy is fine
  (`VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT`).
- **Draw calls:** `vkCmdDrawIndexed` per mesh, no indirect/multi-draw.
- **Descriptors:** allocated once per material per frame-in-flight
  (`GPUMaterial.cpp`) — correct; not per draw. But `getDescriptorSets()` copies
  the vector by value on bind (`GPUMaterial.cpp:158`), and the `materialBound`
  guard (`Renderer.cpp:727`) is a **correctness bug** (descriptors are bound
  only once per material per frame; if materials interleave, the wrong set
  stays bound). No descriptor indexing/bindless.
- **Pipelines:** created once at init, one per shader (`Renderer.cpp`). **No
  `VkPipelineCache`** (`vkCreateGraphicsPipelines(..., VK_NULL_HANDLE, ...)`,
  `Renderer.cpp:538`). Dynamic state used for viewport/scissor/cull (good).
  `vkCmdBindPipeline` redundantly re-issued per mesh even when unchanged.
- **Buffers:** vertex/index buffers are device-local via staging (correct), but
  every mesh does its own staging buffer + `vkQueueWaitIdle`
  (`GPUMesh.cpp:80`, `ADeviceBackend.cpp:233`). No persistent mapping for
  uploads, no ring buffer, no shared staging. UBO is host-coherent and mapped
  once (good, `Frame.cpp`).
- **Synchronization:** `vkQueueWaitIdle` in `endSingleTimeCommands`
  (`ADeviceBackend.cpp:233`) used for every buffer copy, image transition, and
  mipmap chain. `vkWaitForFences` with `UINT64_MAX` between eyes and before
  present. `vkDeviceWaitIdle` only at teardown (fine). No timeline semaphores.

---

## OpenXR bottlenecks

- `xrWaitFrame` → CPU work → `xrBeginFrame` → … → `xrEndFrame` ordering is
  correct, but the render is not pipelined.
- **Per-eye fence wait (CONFIRMED):** after submitting eye 0 the CPU blocks on
  the in-flight fence before recording eye 1 (`Renderer.cpp:219-230`).
- **Pre-composition fence wait (CONFIRMED):** `waitOnImageAvailable == false`
  for OpenXR, so the code waits for the render submission to finish before
  releasing swapchain images / `xrEndFrame` (`Renderer.cpp:236-245`). The GPU
  is fully drained every frame.
- **`xrWaitSwapchainImage` with `XR_INFINITE_DURATION`**
  (`XrSwapchainContext.cpp`) — a missed frame blocks indefinitely.
- Predicted display time is used for `xrLocateViews` and `xrEndFrame`
  (correct), but the **60 FPS sleep** means the app often cannot hit the
  predicted time, forcing the compositor into reprojection.
- **Two separate swapchains** (one per eye) and two render passes; no
  multiview.
- View/pose updates are cheap (`syncViewSet`, `XrSwapchainContext.cpp`) — fine.

**Effect on frame pacing:** frame time is `CPU_record + GPU_execute`
(serialized), not `max(CPU, GPU)`. A stable 72/90 Hz budget is impossible; the
compositor will reproject or drop frames.

---

## Memory and allocation analysis

- **Per-frame heap traffic (CONFIRMED):** `Scene::getMeshes()` vector +
  `shared_ptr` copies (`Scene.cpp:71-79`); `RessourceManager::sync()` three
  full map copies (`RessourceManager.cpp:89-94`); `getDescriptorSets()` vector
  copies; `_capturedViewportEvents` vector growth (`Engine.cpp:360`).
- **Per-object-add heap traffic:** a new `RenderObject`, a new `GPUMesh` per
  mesh, staging buffer + device memory alloc/free, `std::map` node insert in
  `Scene::_objects` (`Scene.cpp:44`).
- Data structures are AoS and pointer-heavy (`shared_ptr<GPUMesh>`,
  `std::map<uint32_t, shared_ptr<...>>`) — cache-unfriendly but secondary to
  the allocation/GPU-upload churn.
- No arenas/pools for transient frame data.

---

## Synchronization analysis

| Call | Location | Impact |
| ---- | -------- | ------ |
| `vkWaitForFences(UINT64_MAX)` between eyes | `Renderer.cpp:221` | Serializes CPU/GPU per eye; ~1 frame stall |
| `vkWaitForFences(UINT64_MAX)` before present (OpenXR) | `Renderer.cpp:238` | Full GPU drain per frame |
| `vkQueueWaitIdle` per buffer copy | `ADeviceBackend.cpp:233` | Full queue drain on every mesh/texture upload |
| `xrWaitSwapchainImage(XR_INFINITE_DURATION)` | `XrSwapchainContext.cpp` | Unbounded block on missed frames |
| Fence-per-frame + semaphores per swapchain | `Frame.cpp` | Reasonable, but `MAX_FRAMES_IN_FLIGHT=2` overlap is defeated by the per-eye waits |

`MAX_FRAMES_IN_FLIGHT = 2` (`EvanPlatform.hpp`) is correct in principle, but
the shared command buffer + per-eye fence wait collapses it to effectively one
frame in flight during stereo.

---

## Optimization roadmap

### P0 — Critical

| # | Problem | Evidence | Why it hurts | Impact | CPU/GPU | Difficulty | Risk | Effort | Benchmark |
| - | ------- | -------- | ------------ | ------ | ------- | ---------- | ---- | ------ | --------- |
| P0.1 | Per-mesh logging in draw loop | `Renderer.cpp:676-745` | ~5 formatted+flushed log lines per mesh per eye | Very high, linear in N | CPU | Easy | Low | 0.5–1 day | Frame time with/without logs at N=100/1000 |
| P0.2 | Remove 60 FPS sleep | Done: `Engine.cpp` (opt-in limiter), `DesktopSwapchainImage.cpp` (FIFO default) | Caps loop; VR can't hit 72/90 Hz; +16 ms latency | High | CPU | Done | Low | 0.5 day | Frame-time histogram; compositor reprojection |
| P0.3 | Per-eye + pre-present fence waits | `Renderer.cpp:219-245` | Serializes CPU/GPU; unstable pacing | High | Both/sync | Medium | Medium | 2–4 days | GPU idle gaps; CPU vs GPU frame time |
| P0.4 | `RessourceManager::sync()` per frame & per add | `RessourceManager.cpp:77-174`; `Engine.cpp:139,166,190,222,238` | Map copies + O(M·E) scans every frame | High | CPU | Easy | Low | 1–2 days | `sync()` call count + time |

### P1 — High impact

| # | Problem | Evidence | Impact | Difficulty | Effort |
| - | ------- | -------- | ------ | ---------- | ------ |
| P1.1 | No culling; all meshes drawn | `Renderer.cpp:675` | GPU + CPU linear in N | Medium | 3–5 days |
| P1.2 | `Scene::getMeshes()` allocates per frame | `Scene.cpp:69` | O(N) allocation/copies per view | Easy | 1 day |
| P1.3 | No batching/instancing; redundant state binds | `Renderer.cpp:675-748` | Draw-call + CPU overhead | Medium–Hard | 1–2 weeks |
| P1.4 | Done: `getDescriptorSets()` returns const reference | `GPUMaterial.hpp:150`; `Renderer.cpp:876,1035` | Allocation per material bind (removed) | Done | Done |
| P1.5 | `vkQueueWaitIdle` per upload | `ADeviceBackend.cpp:233` | Queue drain on every upload | Medium | 3–5 days |
| P1.6 | Add `VkPipelineCache` | `Renderer.cpp:538` | Startup + runtime compile | Easy | 1 day |
| P1.7 | Reduce MSAA / disable blend for opaque | `DeviceContext.cpp:178`; `Renderer.cpp:453` | Bandwidth/ROP | Easy | 1–2 days |

### P2 — Medium impact

- P2.1 Two command buffers per frame to remove inter-eye serialization
  (`Renderer.cpp`/`Frame`).
- P2.2 Move per-object transforms to a GPU instance/SSBO buffer (enables
  P1.3).
- P2.3 `VK_KHR_multiview` for stereo.
- P2.4 Sort draw calls by pipeline/material (and front-to-back for opaque).
- P2.5 Replace `std::map` `_pipelines` with a flat array indexed by shader ID
  (`Renderer.cpp`).

### P3 — Low impact (do later)

- Object pools/arenas for `RenderObject`/`GPUMesh`.
- SoA/contiguous scene storage instead of `shared_ptr` maps.
- `getPipelineLayer()` by value, `getUniformBuffers()` by value.
- Multithreaded command recording.
- Timeline semaphores, bindless/descriptor indexing.
- Fix the `materialBound` correctness bug (`Renderer.cpp`) — done; the bind
  site now resolves the descriptor set once per command buffer.

---

## Instrumentation recommendations (evan-scoped)

- **`Renderer::drawFrame`**: timestamp preprocess, acquire, per-view
  `updateUniformBuffer`/`recordCommandBuffer`/`vkQueueSubmit`, fence waits,
  present. Separates CPU record from GPU wait.
- **Counters** exposed via a debug overlay or log every N frames: visible
  meshes, draw calls (`vkCmdDrawIndexed` count), pipeline binds, descriptor
  binds, `sync()` calls, `GPUMesh` creations/destructions, `vkQueueWaitIdle`
  count.
- **GPU timestamps**: `VkQueryPool` with `vkCmdWriteTimestamp` around the
  render pass per eye, read back with `VK_QUERY_RESULT_WITH_AVAILABILITY_BIT`
  (never blocking).
- **External tools**: RenderDoc (draw-call/state inspection, GPU timing),
  Nsight Graphics / Radeon GPU Profiler (GPU-bound confirmation),
  `VK_LAYER_KHRONOS_validation` + profiles/performance layers.

---

## Expected impact

- **P0.1 (logging):** likely the largest single win; at N=1000 meshes could be
  tens of milliseconds/frame.
- **P0.2 (sleep):** removed the 60 FPS ceiling; VR gains ~16 ms of headroom.
- **P0.3 (fence waits):** converts serialized `CPU+GPU` frame time to roughly
  `max(CPU, GPU)`, stabilizes pacing. Especially large on OpenXR.
- **P0.4 (`sync`):** removes per-frame `O(M·E)` work and map allocations.
- **P1.x:** draw-call and bandwidth reductions; effect grows with N.

No measured numbers exist yet; all figures are engineering estimates from the
static call graph.

---

## Cross-references

- Per-entity logging and the GPU-mesh recreation churn live in `guillaume` —
  see [`PERFORMANCE_AUDIT_GUILLAUME.md`](PERFORMANCE_AUDIT_GUILLAUME.md).
- The logger and `RessourceProvider` that `evan` calls into live in `utility` —
  see [`PERFORMANCE_AUDIT_UTILITY.md`](PERFORMANCE_AUDIT_UTILITY.md).
- The application wrapper that drives `evan::Engine` is `xider` —
  see [`PERFORMANCE_AUDIT_XIDER.md`](PERFORMANCE_AUDIT_XIDER.md).
