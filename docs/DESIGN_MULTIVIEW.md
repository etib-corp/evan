# Design: `VK_KHR_multiview` stereo rendering

Status: implemented (see §10; runtime validation and measurements pending)
Scope: evan (engine, device backends, swapchains, renderer, example shader contract)
Parent: #118 item 3 (roadmap **P2.3**); tracked by issue #145
Depends on: P0-03 per-eye fence removal (landed), P2.2 GPU transforms (landed),
P2.4 draw sorting (landed)

## 1. Goals / non-goals

**Goals**

- When the device and the OpenXR runtime allow it, render both stereo eyes in
  **one render pass / one draw submission**: a single layered swapchain, a
  single framebuffer, one command buffer and one queue submission per frame.
- Select the per-eye matrices in the vertex shader through `gl_ViewIndex`
  (`ViewIndex`), fed from per-view uniform entries.
- Keep the existing two-pass stereo path as a transparent fallback, and keep
  desktop behaviour identical (view mask `0`).
- Preserve P0-03's synchronization contract (one end-of-frame wait in the XR
  path) and P2.2's always-on instance transform layout.

**Non-goals (this change)**

- Desktop stereo / multi-view. GLFW still renders one view; the `viewMask`
  parameter keeps the door open.
- Foveation, per-eye viewport/rate control, selective per-eye rendering.
- Migrating shaders to the per-instance model matrix (P2.2 follow-up): that is
  a prerequisite asset update, not part of multiview. This design only moves
  the **view/projection** pair out of the flat UBO into a two-entry array.

## 2. Current state (verified)

- `XrSwapchainContext` creates one `XrSwapchain` per view configuration with
  `arraySize = 1` and maps each view to its own swapchain
  (`_viewSet[i].swapchainIndex = i`).
- `Renderer::drawFrame` records and submits one command buffer per view, each
  with its own dynamic UBO slot (`updateUniformBuffer(view, i)`), one render
  pass per eye.
- `ASwapchainContext::createRenderPass` has no view mask; it builds a single
  subpass over (color, depth[, resolve]).
- `ADeviceBackend::createImageView` always creates a `VK_IMAGE_VIEW_TYPE_2D`
  view with `layerCount = 1`; `CreateImageProperties` has no layer count.
- `XrDeviceBackend::createLogicalDevice` enables no Vulkan 1.1+ features and
  no device extensions.
- The uniform block is `{ mat4 model; mat4 view; mat4 proj; }` per view slot;
  the descriptor is a dynamic UBO whose range is
  `sizeof(Frame::UniformBufferObject)`.
- `frameSlotCount(viewCount, swapchainCount)` already anticipates "several
  views sharing a single swapchain".
- Example vertex shaders are precompiled SPIR-V; the target contract feeds the
  per-object model matrix through vertex binding 1 (locations 3..6).

## 3. Capability model: two independent gates

Multiview is offered only when **both** gates pass; each gate maps to one
fallback level.

| Gate | Question | Where it is decided | Exposed as |
| ---- | -------- | ------------------- | ---------- |
| Device | `VkPhysicalDeviceMultiviewFeatures::multiview` (plus `VK_KHR_multiview` on Vulkan 1.0, `maxMultiviewViewCount >= 2`) | `XrDeviceBackend` device creation | `ADeviceBackend::supportsMultiview()` |
| Runtime | Does `xrCreateSwapchain` accept `arraySize == viewCount`? | `XrSwapchainContext` construction | `ASwapchainContext::usesMultiview()` |

Fallback matrix:

| Device feature | Runtime layered swapchain | Render path | Shaders |
| -------------- | ------------------------- | ----------- | ------- |
| yes | yes | 1 render pass, view mask `0b11`, 1 layered swapchain | `ViewIndex` shaders |
| yes | no | 2 render passes (today's path), mask `0`, per-eye swapchains | `ViewIndex` shaders (`ViewIndex` ≡ 0) |
| no | n/a | 2 render passes; `ViewIndex` must not be used at all (SPIR-V `MultiView` capability requires the feature) | single-view variant required |

The third row is why §4.4 keeps the shader change minimal and why the plan
ships both variants from one GLSL source (see §6). In practice all XR-capable
drivers (Adreno, Mali, Xclipse, Apple via MoltenVK, desktop GPUs, XR
simulators) support the feature; the mandate from the issue is the **runtime
rejection** fallback (row 2), which the same shaders cover.

An `EVAN_MULTIVIEW=auto|on|off` environment variable (default `auto`)
overrides the selection, mirroring `EVAN_MSAA` / `EVAN_DEBUG_*`: `off` forces
the two-pass path for testing, `on` requires the single-pass path and fails
loudly if it cannot be built.

## 4. Design

### 4.1 Device creation (capability detection, opt-in)

1. New helper on `ADeviceBackend` (shared by backends), run while the physical
   device is known:
   - `vkGetPhysicalDeviceFeatures2` with `VkPhysicalDeviceMultiviewFeatures`
     chained, plus `VkPhysicalDeviceMultiviewProperties::maxMultiviewViewCount`;
   - enumerate device extensions and check `VK_KHR_multiview` when the device
     API version is below 1.1.
   Both queries need an **instance API version of at least 1.1** (the current
   `createInstance` requests `graphicsRequirements.minApiVersionSupported`,
   which can be 1.0.x). Request `maxApiVersionSupported` instead — common
   practice, and required for the core-1.1 query/enable path; if the effective
   version still ends up below 1.1, report `supportsMultiview() == false` (the
   extension-based 1.0 path can be added later if a target ever needs it).
2. `XrDeviceBackend::createLogicalDevice`:
   - when supported, chain `VkPhysicalDeviceMultiviewFeatures{ .multiview =
     VK_TRUE }` into `VkDeviceCreateInfo::pNext` (legal alongside the existing
     `pEnabledFeatures`); the same chain plus `VK_KHR_MULTIVIEW_EXTENSION_NAME`
     is the path if an extension-based Vulkan 1.0 target is added later;
   - store the result and override `supportsMultiview()`.
3. `DesktopBackend` also queries and enables the feature when available.
   Desktop render passes keep a zero view mask, but the feature must be on
   because the shared shader contract uses the `ViewIndex` input, whose SPIR-V
   `MultiView` capability requires it. A device without the feature falls back
   to the two-pass path and needs the single-view shader variant (§4.4).

### 4.2 Render pass

`ASwapchainContext::createRenderPass` gains a trailing parameter
`uint32_t viewMask = 0`:

- `viewMask == 0`: exactly today's render pass (no `pNext` chain) — desktop
  and the fallback path are unchanged by construction.
- `viewMask != 0`: chain a `VkRenderPassMultiviewCreateInfo` with
  `subpassCount = 1`, `pViewMasks = &viewMask`, and
  `correlationMaskCount = 1`, `pCorrelationMasks = &viewMask` (a hint: both
  eyes are spatial neighbours, letting tilers schedule layers together).
  `pViewOffsets` is unused (`dependencyCount = 0`).
- Multiview is all-or-nothing per render pass; the mask is
  `(1u << viewCount) - 1`, capped at `kMaxMultiviewViews = 2` (stereo).
- XR stereo passes `0b11`; every existing call site keeps the default.

The caller only passes a non-zero mask when the device feature is enabled —
`VkRenderPassMultiviewCreateInfo` requires all masks to be `0` otherwise.

### 4.3 Swapchain, attachments and framebuffers

**Ordering inside `XrSwapchainContext`.** The mode must be known before the
render pass is created, and the runtime's answer is only known when
`xrCreateSwapchain` returns. Reorder the constructor to:

1. enumerate view configurations and decide the *tentative* mode:
   `backend.supportsMultiview() && viewCount == 2 && both configurations have
   equal width/height/sampleCount` (strict for phase 1; a max-rect policy can
   come later);
2. create the swapchain handle(s): in tentative multiview mode a **single**
   swapchain with `arraySize = viewCount`; on `xrCreateSwapchain` failure log,
   mark multiview disabled and create the per-eye handles instead;
3. create the render pass with the mask matching the actual result;
4. build the `XrSwapchainImage` wrappers; map the `ViewSet`:
   `swapchainIndex = 0` for every view in multiview mode, `i` otherwise.

`recreateSwapchain` mirrors the same creation steps (mode is sticky for the
session; the render pass already exists and is reused). Note that
`syncViewSet` currently rewrites `_viewSet[i].swapchainIndex = i` on every
frame; the mapping must become mode-aware there as well (one shared helper),
otherwise the single-swapchain mapping is undone after the first frame.

**Layered attachments** (`XrSwapchainImage` reads `arraySize` from
`createInfo`):

- swapchain image views: `VK_IMAGE_VIEW_TYPE_2D_ARRAY`, `baseArrayLayer = 0`,
  `layerCount = arraySize`;
- engine-owned MSAA color image (`ResolveToSwapchain`) and depth image:
  `arrayLayers = arraySize`, matching array views;
- framebuffers: `VkFramebufferCreateInfo::layers = arraySize`.
- With `arraySize == 1` every creation path collapses to today's code.

`ADeviceBackend` plumbing, default-compatible:

- `CreateImageProperties` gains `uint32_t _arrayLayers = 1`;
- `createImageView(..., uint32_t mipLevels, uint32_t layerCount = 1)` creates a
  `2D_ARRAY` view when `layerCount > 1` and a `2D` view otherwise.

**Projection layer views.** `updateProjectionLayerViews` keeps one
`XrCompositionLayerProjectionView` per eye. In multiview mode both reference
the single swapchain with `subImage.imageArrayIndex = view index` (0 left,
1 right) and the shared `imageRect`; in fallback mode they reference their own
swapchain exactly as today. The loop must resolve each view through
`_viewSet[view].swapchainIndex` (`_swapchainImages` has one entry in
multiview mode, not one per eye), not through the view index directly.

**MSAA.** `selectMsaaSamples` is unchanged; only the attachment shape is
layer-aware. If a runtime hands out multisampled layered swapchains it maps to
`SwapchainImageAttachment` as before.

### 4.4 Uniforms and the shader contract

The dynamic UBO keeps one slot per *view* today. Multiview needs **both eyes
addressable from one bind**, selected by the rasterized layer. The slot layout
becomes:

```
UniformBufferObject (one dynamic-UBO slot)              // sizeof = 320
├─ mat4 model                             offset   0   // identity until per-instance shading lands
└─ ViewUniform views[2]  (stride 128)     offset  64
   ├─ [0] { mat4 view; mat4 proj; }       offset  64
   └─ [1] { mat4 view; mat4 proj; }       offset 192
```

Vertex shader (all three example shaders, one source):

```glsl
#extension GL_EXT_multiview : require

struct ViewUniform { mat4 view; mat4 proj; };
layout(set = 0, binding = 0, std140) uniform UBO {
    mat4 model;
    ViewUniform views[2];
} ubo;

void main() {
    // ViewIndex is the current layer in a multiview render pass and 0 in a
    // single-view render pass (Vulkan spec, Built-In Variables/ViewIndex).
    ViewUniform view = ubo.views[gl_ViewIndex];
    vec4 world = ubo.model * vec4(inPosition, 1.0);   // + per-instance model (binding 1)
    gl_Position = view.proj * view.view * world;
}
```

Slot content and binding per mode — this is what "per-view matrices from the
existing per-view uniform slots" means operationally:

| Mode | Slot written | Content | Dynamic offset | `gl_ViewIndex` |
| ---- | ------------ | ------- | -------------- | -------------- |
| Multiview | slot 0 | `views[0] = eye 0`, `views[1] = eye 1` | `0` | `0` / `1` per layer |
| Fallback | slot `i` | `views[0] = eye i` (`views[1]` unused) | `i * alignedSize` | `0` (spec-guaranteed) |

So in the fallback each eye keeps writing into its own slot exactly like
today; the shader reads index 0 of the bound slot. No push constants, no
second shader entry point, one command-buffer recording path for both modes.

- `sizeof(UniformBufferObject)` drives the descriptor range in
  `GPUMaterial.cpp`; changing the struct keeps the range correct
  automatically (320 bytes).
- `Frame::getUniformBufferAlignedSize()` keeps its formula
  (`alignUp(sizeof(UBO), minUniformBufferOffsetAlignment)`).
- `Renderer::updateUniformBuffer` keeps its fallback meaning; a sibling
  `updateMultiviewUniformBuffer` (or an overload taking the view range) writes
  both entries into slot 0.
- Shader assets: the example GLSL sources now live in-tree under
  `examples/GLFW-linux-base/shaders/src/` and the committed SPIR-V is rebuilt
  from them (`glslc --target-env=vulkan1.0`). guillaume/xider-facing samples
  must follow the same contract. Because the shared shader uses `ViewIndex`,
  the multiview device feature is enabled on every device that offers it; the
  single-view variant for feature-less devices is still future work (§9 Q3).

**Alternatives considered and rejected**

- *Push-constant view index* (`views[gl_ViewIndex + pc.viewOffset]`): works,
  but adds a vertex-stage push constant range and a uniform layout rule for a
  path the per-slot scheme already covers.
- *Re-interpreting per-view slots as a fixed-stride array* (`views[gl_ViewIndex]`
  where elements live `alignedSize` apart): the GLSL array stride is fixed
  (128 B) while `minUniformBufferOffsetAlignment` can be up to 256 B; the
  layouts cannot be reconciled.
- *Two render passes with masks `0b01`/`0b10` to recycle `ViewIndex`*: still
  requires layered attachments and does not run on a runtime that rejects
  `arraySize == 2`, i.e. it does not serve the fallback.

### 4.5 Renderer

`Renderer::drawFrame` branches once on `swapchainContext.usesMultiview()`:

- **Multiview branch.** Acquire the single swapchain image (the existing loop
  over `swapchainCount == 1` already does this), write both eyes into UBO
  slot 0, reset command buffer 0, record **one** command buffer over the single
  layered framebuffer, submit **once** with the in-flight fence, then run the
  existing end-of-frame wait (XR) and the single present/release.
  `updateProjectionLayerViews` + `xrEndFrame` are unchanged in shape.
- **Fallback branch.** Today's per-view loop, untouched.

`recordCommandBuffer` adjustments (both modes share it):

- it receives the views the pass covers (one or both). Culling keeps a mesh
  when **any** view's frustum intersects its bounds (conservative, no false
  culls); depth sorting uses the eye midpoint (the existing buckets make this
  negligible); the dynamic UBO offset is `0` in multiview and
  `viewSlot * alignedSize` in fallback.
- bounds, draw list, batching and indirect recording are computed once for the
  pass; in multiview that halves culling, sort-key and state-tracking work
  versus two passes.
- `viewOwnsInFlightFence` stays for the fallback; the multiview branch submits
  once so the single submission owns the fence — the P0-03 contract (one
  end-of-frame GPU wait before release) is preserved and becomes simpler.

Per-frame resources: `frameSlotCount(viewCount=2, swapchainCount=1) == 2`
slots are still allocated; multiview only uses slot 0. No allocation change is
required (an optional follow-up can stop allocating per-eye uniforms when
multiview is active).

### 4.6 Fallback

- **Runtime rejects `arraySize == 2`** (the case the issue mandates):
  `XrSwapchainContext` transparently falls back to per-eye swapchains, mask
  `0`, `swapchainIndex = i`; the renderer takes the existing two-pass branch.
  The `ViewIndex` shaders stay valid (`ViewIndex` ≡ 0, each eye reads its own
  slot) — no asset swap, no visible behaviour change except the missing
  speedup.
- **Device lacks the multiview feature**: `supportsMultiview()` is false; the
  engine cannot run `ViewIndex` shaders, so applications must load the
  single-view shader variant (§4.4). This path keeps the current two-pass
  renderer; see open question Q3.

## 5. File-by-file change map

| File | Change |
| ---- | ------ |
| `headers/evan/ADeviceBackend.hpp`, `sources/ADeviceBackend.cpp` | `supportsMultiview()` query helper; `createImageView` layer count; `CreateImageProperties._arrayLayers` |
| `headers/evan/openxr/XrDeviceBackend.hpp`, `sources/openxr/XrDeviceBackend.cpp` | feature/extension query, feature enable at device creation, capability flag |
| `headers/evan/ASwapchainContext.hpp`, `sources/ASwapchainContext.cpp` | `viewMask` parameter + `VkRenderPassMultiviewCreateInfo`; `virtual bool usesMultiview()` (default false) |
| `headers/evan/openxr/XrSwapchainContext.hpp`, `sources/openxr/XrSwapchainContext.cpp` | mode selection, single layered swapchain, `imageArrayIndex`, `swapchainIndex` mapping, recreation |
| `headers/evan/openxr/XrSwapchainImage.hpp`, `sources/openxr/XrSwapchainImage.cpp` | layered views, color/depth `arrayLayers`, framebuffer `layers` |
| `headers/evan/Frame.hpp`, `sources/Frame.cpp` | `UniformBufferObject` becomes `{ model; views[2]; }`; multiview slot write helper |
| `headers/evan/Renderer.hpp`, `sources/Renderer.cpp` | single-pass branch, view-range-aware recording, multiview UBO upload |
| `examples/*/shaders` (GLSL sources + SPIR-V) | `gl_ViewIndex` indexing, rebuilt binaries; single-view variant |
| `tests/sources/` | new `test_multiview.cpp` (mask derivation, UBO offsets, mode selection helpers, layer/image index mapping) |
| `docs/PERFORMANCE_AUDIT_EVAN.md`, `CHANGELOG.md` | status update when the implementation lands |

## 6. Sequencing and acceptance

Land last in the P2 sequence (per the parent issue). Each phase builds and is
independently testable:

1. **Capability plumbing** — query helper, feature enable, `supportsMultiview()`,
   `EVAN_MULTIVIEW` parsing. No behaviour change. *Acceptance:* built on
   Linux/Windows/macOS/Android; feature appears in the startup log;
   `EVAN_MULTIVIEW=off` is a no-op.
2. **Render pass + image plumbing** — `viewMask` parameter, layered
   `createImageView`/`CreateImageProperties`, all defaults preserved.
   *Acceptance:* desktop and XR two-pass rendering visually unchanged;
   validation layers clean.
3. **Uniform + shader contract** — `views[2]` block, fallback writes per-eye
   slots; rebuild example SPIR-V. *Acceptance:* two-pass XR and desktop
   render correctly with the new shader contract.
4. **XR layered swapchain + renderer single-pass branch (one change)** — mode
   selection, single swapchain, projection layer indices, recreation, plus the
   collapse of the renderer loop (one recording, one submission, culling
   across eyes). These land together on purpose: once the render pass carries
   a `0b11` mask, a per-eye recording would broadcast each pass into **both**
   layers, so the recorder must switch in the same change.
   *Acceptance:* on a capable runtime, a RenderDoc capture shows exactly one
   render pass per frame with two populated layers; both eyes are visually
   correct; validation layers are clean.
5. **Docs/measurements** — update the performance audit with the numbers from
   the issue's PoC, changelog entry.

Until phase 4 lands, multiview stays disabled by default: the
`EVAN_MULTIVIEW=auto` gate simply never selects the single-pass path while the
recorder cannot drive it.

## 7. Test plan

**Unit (host, no runtime)**

- `viewMask` derivation: mono → `0`, stereo → `0b11`, view count above
  `kMaxMultiviewViews` → fallback.
- Mode selection: (feature, runtime acceptance, rects equal) → single/per-eye.
- `UniformBufferObject` layout: offsets 0/64/192 and `sizeof == 320`
  (compile-time `static_assert` on the size, runtime checks on the offsets);
  slot stride alignment math unchanged.
- Projection layer mapping: both views → same swapchain, `imageArrayIndex`
  0/1; fallback unchanged (extends `test_viewset` / `test_swapchain_view`).
- Frame slots: existing `MultiViewSharingSingleSwapchain` coverage still
  passes; fence ownership for the single submission.

**Integration (manual / CI runner where available)**

- Desktop GLFW example: regression check, mask `0`, layered plumbing inert.
- OpenXR stereo on a capable runtime: validation clean, one render pass in a
  capture, both eyes correct, `EVAN_DEBUG_DRAW_LOG` draw-call and bind counts
  halve.
- **Forced fallback** (`EVAN_MULTIVIEW=off`, and at least one runtime known to
  reject `arraySize == 2`, as the issue requires): identical image, no
  validation errors, `usesMultiview() == false` in logs.

**Measurement (issue PoC)**

- GPU frame time / render-pass duration from GPU timestamps or RenderDoc,
  two-pass vs multiview, fixed scene.
- Vertex and fragment invocation counts per frame at a fixed scene; expected:
  vertex shading still runs per layer, fragment work unchanged in theory, so
  the measured win is command recording, state tracking, culling and
  render-pass overhead — report the real numbers instead of assuming a
  halving.

## 8. Risks and mitigations

- **Large surface area / driver variance** — land behind `EVAN_MULTIVIEW=auto`
  in incremental phases, with `off` as the universal escape hatch; gate on
  both device and runtime capability instead of assuming support.
- **Instance API version bump** — requesting `maxApiVersionSupported` instead
  of `minApiVersionSupported` affects instance creation on every backend path;
  keep it a separate, first-phase commit and verify each target runtime
  (Linux, Windows, Android) still creates its session.
- **Runtime rejects layered swapchains** — the mandated fallback (§4.6 row 2);
  tested explicitly with `EVAN_MULTIVIEW=off` and a rejecting runtime.
- **P0-03 overlap (per-eye sync)** — multiview collapses the fence story to a
  single submission; do not reintroduce per-view fences. Coordinate the fence
  cleanup so the two changes do not diverge.
- **P2.2 overlap (GPU transforms)** — both touch the same UBO block; land
  multiview after the instance-transform shader update and keep `model` in the
  block until it is removed engine-wide.
- **MSAA / resolve interaction** — layered color/depth/resolve attachments are
  created from the same `arraySize`; validate on a multisampled runtime if one
  is available, otherwise restrict testing to the single-sampled path.
- **`minUniformBufferOffsetAlignment`** — the slot stride stays aligned via
  `Frame`; the shader indexes inside the bound range, never across slots.
- **Shader asset distribution** — the shader sources are out of tree
  (guillaume/xider and the examples' precompiled SPIR-V): the contract change
  must be released in lockstep and the single-view variant compiled from the
  same source.
- **Layered + `ResolveToSwapchain`** — a runtime that recommends MSAA and a
  layered swapchain exercises both features at once; keep the mode selection
  strict and document that a failure of framebuffer creation for layered MSAA
  falls back to two-pass (mode selection happens before the render pass).

## 9. Open questions

1. **Fencing for the single submission**: keep the host-side end-of-frame wait
   (`vkWaitForFences`) exactly as P0-03 landed it, or attach the fence to the
   single submit and wait right before `xrReleaseSwapchainImage`? Current
   proposal: no change to the wait placement — only fewer submissions.
2. **Culling policy in multiview**: "any frustum intersects" is proposed;
   alternatively render with the union eye and accept a slightly larger draw
   set. Confirm with a measurement before over-engineering.
3. **Device without the multiview feature**: ship the pre-change two-pass
   shaders as the supported variant (documented), or declare such devices
   unsupported for the new shader generation? Recommendation: keep the
   single-view variant from the same GLSL source, minimal cost.
4. **View configuration rect mismatch between eyes**: strict fallback for now;
   a shared max-rect policy may be worth revisiting if a runtime reports it.

## 10. Implementation status (2026-10-09)

Landed:

- Device capability query (`ADeviceBackend::queryMultiviewSupport`,
  `supportsMultiview()`) and feature enablement on `XrDeviceBackend` and
  `DesktopBackend`; `EVAN_MULTIVIEW=auto|on|off` override.
- `createRenderPass(..., viewMask = 0)` with the
  `VkRenderPassMultiviewCreateInfo` chain; `usesMultiview()` on the swapchain
  contexts.
- Layered resources: `CreateImageProperties::_arrayLayers`,
  `createImageView(..., layerCount)`, layer-aware color/depth attachments and
  framebuffers.
- `XrSwapchainContext`: single `arraySize == viewCount` swapchain with
  per-eye `imageArrayIndex`, mode-aware `ViewSet` mapping and recreation, and
  the runtime-rejection fallback to one swapchain per view.
- `Frame::UniformBufferObject` is `{ model; views[2]; }`; the renderer writes
  one slot per eye in the fallback and both eyes into slot 0 in multiview.
- `Renderer`: single-pass branch (one recording, one submission, one fence)
  and conservative culling across the eyes covered by the pass.
- Example GLSL sources committed under
  `examples/GLFW-linux-base/shaders/src/` and SPIR-V rebuilt with
  `gl_ViewIndex`; unit tests for the view mask and the uniform layout.

Still to do on hardware:

- Run the OpenXR sample on a runtime that accepts `arraySize == 2` and on one
  that rejects it (validation clean, both eyes correct, one render pass per
  frame in a capture).
- Collect the PoC numbers from §7 and record them in
  `docs/PERFORMANCE_AUDIT_EVAN.md`.
- Decide Q3 (single-view shader variant) if a device without the multiview
  feature ever needs to be supported.
