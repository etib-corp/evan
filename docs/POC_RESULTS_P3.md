# P3 proof-of-concept results — N=100 vs N=1000

Status: recorded run on the fixed build (desktop single view, macOS / MoltenVK). This file
is the measurement record the WS4/WS5/WS6 gates of
[`DESIGN_P3_OPTIMIZATIONS.md`](DESIGN_P3_OPTIMIZATIONS.md) §4 are decided on.
It is the expanded `summary.md` produced by the harness, kept next to the
design so the raw windows stay reviewable.

Date: 2026-10-29 (numbers re-recorded after the shutdown/validation fixes
described in §7; the first record of the same day was taken on the pre-fix
build and is superseded). §10 (2026-11-19) is the latest record: it re-runs
the harness on the post-#175–#177 build and adds the Release/validation-off
control.
Harness: [`scripts/run-poc.sh`](../scripts/run-poc.sh)
Workload: [`examples/GLFW-macos-poc`](../examples/GLFW-macos-poc) with
`EVAN_POC_MESHES=100` / `EVAN_POC_MESHES=1000`

## 1. Environment

| Field | Value |
| ----- | ----- |
| Machine | Apple M3 |
| CPU | Apple M3 |
| GPU | Apple M3 (vendor `0x106b`, Metal 4) |
| OS | macOS 27.2 (build 26B123), Darwin 27.2.0, arm64 |
| Vulkan | MoltenVK, Vulkan SDK 1.4.304.1 |
| Build | Debug (`CMAKE_BUILD_TYPE=Debug`), `VK_LAYER_KHRONOS_validation` active |
| Window | 800×600, default MSAA (1 sample), single view |
| Instrumentation | `EVAN_STATS=1`, `EVAN_STATS_INTERVAL=60`, GPU timestamps on |

## 2. Method

- **Workload.** One shared local-space cube mesh; each of the N objects is its
  own `Primitive`, so it gets its own registry entry, its own `GPUMesh` (two
  device allocations: vertex and index buffer) and its own draw call. The grid
  is fitted to the live view and placed with `Engine::setObjectTransform`.
- **Run.** 420 frames per N = 7 report windows of 60 frames. Windows 1–2 are
  warm-up; the harness also drops any remaining window whose `mesh_churn` is
  not `+0/-0` (the scene is still changing there).
- **Kept windows.** 5 per N (windows 3–7). Window 1 reports the startup
  allocations (`mesh_churn +N/0`, `uploadwaits=1`) and is never averaged.
- **Units.** `cpu_ms`/`gpu_ms` are per-frame averages as printed. The counters
  (`meshes`, `draws`, `sync`, `mesh_churn`, `uploadwaits`) are **sums over the
  window**, so they are divided by the window's `frames=` value; the tables
  below show per-frame values.
- **Log level.** `UTILITY_LOG_LEVEL=info` and `NO_COLOR=1`: the debug default
  logs every vertex, and the release default suppresses the `info` report
  window entirely.

## 3. Averaged results

| N | windows | frames | rec (ms) | submit (ms) | wait (ms) | ubo (ms) | pres (ms) | gpu pass0 (ms) | draws calls | pso | ds | mesh_churn | uploadwaits | vis | culled |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 100 | 5 | 300 | 0.98 | 7.00 | 0.02 | 0.01 | 0.03 | 0.17 | 101 | 1 | 1 | +0/-0 | 0 | 101 | 0 |
| 1000 | 5 | 300 | 5.80 | 2.24 | 0.02 | 0.01 | 0.02 | 0.62 | 1000 | 1 | 1 | +0/-0 | 0 | 1000 | 0 |

Derived: `rec` per recorded draw is 9.8 µs at N=100 and 5.8 µs at N=1000;
`rec` grows 5.9× for a 10× object count because the per-draw cost is lower once
the pipeline, the descriptor set and the buffers are already bound.

## 4. Per-window detail

Every kept row is a 60-frame window; counters are already divided by 60.
`pass0` is the GLFW single view.

### N=100

| window | rec | submit | wait | ubo | pres | gpu pass0 | calls | pso | ds | churn | uploadwaits |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 3 | 0.91 | 7.09 | 0.02 | 0.01 | 0.03 | 0.26 | 101 | 1 | 1 | +0/-0 | 0 |
| 4 | 0.92 | 7.10 | 0.02 | 0.01 | 0.03 | 0.16 | 101 | 1 | 1 | +0/-0 | 0 |
| 5 | 0.75 | 7.30 | 0.01 | 0.01 | 0.03 | 0.14 | 101 | 1 | 1 | +0/-0 | 0 |
| 6 | 1.18 | 6.77 | 0.02 | 0.02 | 0.04 | 0.15 | 101 | 1 | 1 | +0/-0 | 0 |
| 7 | 1.12 | 6.75 | 0.02 | 0.02 | 0.04 | 0.13 | 101 | 1 | 1 | +0/-0 | 0 |
| avg | 0.98 | 7.00 | 0.02 | 0.01 | 0.03 | 0.17 | 101 | 1 | 1 | +0/-0 | 0 |

### N=1000

| window | rec | submit | wait | ubo | pres | gpu pass0 | calls | pso | ds | churn | uploadwaits |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 3 | 5.95 | 2.06 | 0.02 | 0.01 | 0.03 | 0.60 | 1000 | 1 | 1 | +0/-0 | 0 |
| 4 | 5.74 | 2.31 | 0.02 | 0.01 | 0.02 | 0.63 | 1000 | 1 | 1 | +0/-0 | 0 |
| 5 | 5.75 | 2.30 | 0.02 | 0.01 | 0.02 | 0.60 | 1000 | 1 | 1 | +0/-0 | 0 |
| 6 | 5.83 | 2.20 | 0.02 | 0.01 | 0.02 | 0.58 | 1000 | 1 | 1 | +0/-0 | 0 |
| 7 | 5.75 | 2.31 | 0.02 | 0.01 | 0.02 | 0.68 | 1000 | 1 | 1 | +0/-0 | 0 |
| avg | 5.80 | 2.24 | 0.02 | 0.01 | 0.02 | 0.62 | 1000 | 1 | 1 | +0/-0 | 0 |

Dropped windows: window 1 of each run (`mesh_churn +100/0` and `+1000/0`,
`uploadwaits=1`) plus the configured warm-up window 2.

## 5. Spread across runs

Three runs of the harness on this machine after the fixes (5 steady-state
windows each, same configuration). Averages per run:

| Metric | N=100 | N=1000 |
| ------ | ----- | ------ |
| `rec` | 0.87–0.98 ms | 5.80–6.23 ms |
| `submit` | 6.82–7.10 ms | 0.37–2.24 ms |
| `wait` | ≤0.22 ms | ≤0.03 ms |
| `gpu pass0` | 0.17–0.47 ms | 0.33–0.62 ms |
| `draws calls` | 100–101 | 1000–1001 |
| `pso` / `ds` | 1 / 1 | 1 / 1 |
| `mesh_churn` | +0/-0 | +0/-0 |
| `uploadwaits` | 0 | 0 |

`rec`, the draw/bind counters and the churn/upload-wait counters are stable.
`submit` is not: it still swings between runs on this machine (MoltenVK
queue/present blocking, unrelated to the object count), so use `rec` itself
rather than its share of the frame. `gpu pass0` moves in ~0.1 ms steps.

`draws calls` reads N+1 when the pointer enters the window before the
measurement: the engine registers a debug ray mesh on the first mouse-motion
event even though this sample sets no event callback.

`rec` is CPU-bound and contention-sensitive: the three runs above were taken
on an idle host (load average < 1), and the same harness reports 2.7 ms and
14.1 ms at N=100/N=1000 when the machine runs at a load average of ~11. Record
on an idle machine, and compare runs from the same session.

## 6. Decision

The decisions below are the ones this record was written for (2026-10-29,
pre-optimization build). They were re-taken after #175–#177 in §10.

| Workstream | Trigger (design §4) | Measured | Decision |
| ---------- | ------------------- | -------- | -------- |
| WS4 — parallel command recording | XR stereo per-view `rec` > ≈15 % of the frame CPU budget (≈1.7 ms at 90 Hz) | desktop single view, no per-view split; `rec` 0.76–1.19 ms at N=100 and 6.2–12.9 ms at N=1000, growing linearly with the draw count | **keep serial** — a desktop run cannot answer the trigger; re-run this harness on the OpenXR stereo build |
| WS5 — timeline semaphores | `wait` / `renderwait` dominate the frame, or a transfer queue / async compute is scheduled | `wait` ≤0.5 ms (≤10 % of the frame), `renderwait` absent on GLFW, one submission per frame | **keep binary fences** — no ordering need is visible |
| WS6 — bindless / descriptor indexing | `ds` grows with the material count on real content | `ds` = 1 per frame at both N (one material, one bind per run) | **keep per-material sets** — binds are independent of the object count here |

## 7. Caveats and next steps

- **Desktop only.** WS4 is decided by per-view recording cost in stereo;
  the OpenXR build (`OpenXR-linux-base` / a macOS XR build) is the
  configuration to measure next. The workload logic is platform-independent,
  so the same knobs can be mirrored there.
- **GPU timer granularity.** `pass0` moves in ~0.1 ms steps around a ~0.3 ms
  pass, so treat it as an order-of-magnitude signal. The cubes are small on
  screen (~25 px at N=100, ~13 px at N=1000 because the grid is fitted to the
  frustum); a RenderDoc capture at N=1000 is the intended cross-check when
  GPU behaviour matters.
- **Culling (settled 2026-11-17).** Back-face culling is now static pipeline
  state: `VK_CULL_MODE_BACK_BIT` with
  `VK_FRONT_FACE_COUNTER_CLOCKWISE`, baked into every pipeline.
  `VK_DYNAMIC_STATE_CULL_MODE` stays out of the dynamic-state list because the
  device does not enable `VK_EXT_extended_dynamic_state`, so the old
  undefined-at-draw state and its VUID are gone; only viewport and scissor are
  dynamic.
  The winding was settled from the projection itself, not assumed. `ViewF`
  flips Y for Vulkan NDC, and that flip cancels the framebuffer's Y-down
  convention, so the counter-clockwise outside-facing winding of the sample
  cube stays counter-clockwise in framebuffer coordinates and is front-facing
  with `VK_FRONT_FACE_COUNTER_CLOCKWISE`
  (`TestRenderSettings.SampleCubeOutwardWindingIsFrontFacing` reproduces the
  projection and the sample cube). `EVAN_CULL_MODE=none|back|front` selects
  the policy for A/B runs, and `CullMode::None` per material (or
  `Engine::setMaterialCullMode`) is the double-sided escape hatch.
  Validation on macOS/MoltenVK: the N=1000 cube grid renders identically
  with `none` and `back` (483 differing bytes out of 7.68 M in a full-window
  capture, i.e. cursor/timing noise), the `EVAN_POC_QUAD=1` single-sided quad
  renders with its `CullMode::None` override and disappears without it, the
  `culled` mesh counter stays 0, and validation reports no VUIDs. `pass0` at
  N=1000 is 0.27 ms with `back` and 0.23 ms with `none` in the final 8-window
  run (repeated 4-window runs ranged 0.28–0.51 ms vs 0.23–0.49 ms), so the
  `≤ 0.45 ms` acceptance holds but the delta stays inside the GPU timer's
  noise: the cubes are ~13 px, and the CPU records draws ~10× more
  expensively than the GPU executes them, so the double-sided fill this
  removes only becomes a wall once #175 collapses the draw count. With #175
  in (§10) the draw count has collapsed and it still is not: `pass0` at
  N=1000 is 0.09 ms (merged scene) to 0.27 ms (1000 distinct meshes) against
  `rec` 0.76–2.50 ms, so the CPU recorder stays the wall.
- **Fixed while recording this.** The shutdown abort (`std::system_error:
  mutex lock failed`), the 30 objects leaked past `vkDestroyDevice`, the
  dynamic-cull-mode violations and the descriptor-set-free violation are gone
  in this build: three consecutive empty-scene runs and the N=100/N=1000 runs
  finish with `rc=0` and zero validation errors.
- **One remaining input.** Re-run `scripts/run-poc.sh` on the stereo build and
  append the per-view `rec` values here; that is the only missing input for
  WS4.


## 8. Geometry sharing and instanced merge (2026-11-06)

§1–§7 measure the per-object-`GPUMesh` build: N objects uploaded the same cube
N times, so every copy got distinct vertex and index buffers and the renderer
could not merge their draws. This section records the same workload after the
geometry cache and the shader-gated instanced merge landed
([`DESIGN_P3_OPTIMIZATIONS.md`](DESIGN_P3_OPTIMIZATIONS.md) §3.1a).

**What changed.**

- `DeviceContext` owns a `GeometryCache` keyed by a hash of the mesh content
  (vertex attributes, index data, both counts) plus the material ID.
  `RenderObject` builds its meshes through it, so objects made from the same
  mesh share one `GPUMesh` and therefore one vertex and one index buffer.
  `RenderObject::updateMeshes` detaches a shared mesh before writing into it
  (copy-on-write), and drops the cache entry of a mesh it rewrites in place.
- The renderer merges a run of draws only when the bound vertex shader
  declares the per-instance model matrix (SPIR-V `Input` variables at locations
  3–6). `default.vert` and `text.vert` do not, so they are never merged.
- `RenderObject` stores its own model matrix and the renderer reads it from
  there: one `GPUMesh` may now serve several objects, each with its own
  transform. Merging is enabled by default (`Renderer::setInstancingEnabled`).

**Method.** Same harness, same workload and same machine as §1, Debug build
with validation. The runs below are interleaved (old build, new build, old
build, new build) inside one session so that host load affects both sides
equally; the host was *not* idle (load average ≈9) and `rec` was inflated by
roughly 70 % relative to §3 for both builds. The stepped decomposition in
§8.2 was taken earlier in the session at load average < 2.

### 8.1 Interleaved A/B (Debug, load average ≈9)

| N | build | `rec` (ms) | `gpu pass0` (ms) | draws/frame | `inst` | `pso` | `ds` | start `mesh_churn` | steady `mesh_churn` | `uploadwaits` |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 100 | before | 1.77–1.79 | 0.19–0.20 | 101 | 0 | 1 | 1 | +101/-0 | +0/-0 | 0 |
| 100 | after | 1.27–1.28 | 0.19–0.20 | 1 | 1 | 1 | 1 | +1/-0 | +0/-0 | 0 |
| 1000 | before | 16.6–19.4 | 0.65–1.01 | 1000 | 0 | 1 | 1 | +1000/-0 | +0/-0 | 0 |
| 1000 | after | 11.2–11.6 | 0.21–0.32 | 1 | 1 | 1 | 1 | +1/-0 | +0/-0 | 0 |

`rec` improves by 1.40× at N=100 and 1.45× at N=1000 in the same session; the
same ratio was measured on an idle host (9.81 → 6.78 ms at N=1000, §8.2), so
the gain is not a load artifact. `gpu pass0` also drops at N=1000 because
999 fewer draws are encoded per frame.

The `N+1` draw of §3 is still visible at N=100: the debug ray registered by the
first mouse-motion event is a second, non-mergeable mesh (one extra draw), which
is the `draws ≤ 2` the issue expects.

### 8.2 Where the remaining `rec` goes (N=1000, Debug, idle host)

Three builds of the same scene measured back to back:

| build | `rec` (ms) | draws/frame | vertex + index binds/frame |
| --- | --- | --- | --- |
| before (N `GPUMesh`, no merge) | 9.81 | 1000 | 2000 |
| geometry cache only (merge off) | 7.67 | 1000 | 0 |
| geometry cache + instanced merge | 6.78 | 1 | 0 |

The cache alone saves 2.1 ms by removing 2000 buffer binds per frame, merging
saves a further 0.9 ms by removing 999 `vkCmdDrawIndexed` calls. The remaining
6.8 ms is *not* per-draw state: it is the per-mesh draw-list build (world-space
bounds, culling, material/shader/pipeline resolution, sort key, instance matrix
write) at `-O0`. `rec` therefore no longer contains a per-draw term — the
`5.36 µs` of §3 were dominated by bookkeeping that this change does not touch,
not by the draw command.

This is visible in an optimized build, where that bookkeeping almost
disappears (Release, same machine, `pass0` still in the table above):

| N | build | `rec` (ms) | draws/frame | `gpu pass0` (ms) |
| --- | --- | --- | --- | --- |
| 100 | before | 0.05 | 101 | 0.26 |
| 100 | after | 0.04–0.05 | 1 | 0.19 |
| 1000 | before | 0.41–0.46 | 1001 | 0.63–0.72 |
| 1000 | after | 0.25 | 1 | 0.20 |

### 8.3 Acceptance

| Criterion | Result |
| --- | --- |
| N=1000 `draws` ≤ 2 | 1 (2 when the debug ray is registered) |
| N=1000 `instancedDraws` ≥ 1 | 1 per frame |
| N=1000 `rec` ≤ 1.0 ms | 0.25 ms on Release; 6.8–11.6 ms on Debug (1.45× better than before, §8.2 explains the remainder) |
| `pso` = 1, `ds` = 1 | 1 / 1 at both N |
| startup `mesh_churn +1/0` | +1/-0 at both N (was +100/-0 and +1000/-0) |
| steady `mesh_churn +0/-0`, `uploadwaits` = 0 | +0/-0, 0 |
| image identical to the pre-change capture | 83 of 2 022 400 pixels differ (0.004 %), all inside a 16×27 px patch at the top centre of the frame in the old capture only; the rendered cubes are pixel-identical |
| shader without instance inputs is never merged | unit tests on the capability query (`tests/sources/test_shader_instancing.cpp`); `default.vert` / `text.vert` scenes keep one draw per mesh |

## 9. Draw-list build and per-draw recording (2026-11-13)

§8 left the per-draw workload unmeasured: sharing one `GPUMesh` between the
objects made the draw count collapse, so the scene never exercised the
per-draw path that real content hits. This section records the same harness
with the `EVAN_POC_UNIQUE_MESHES` knob (one distinct mesh per cube, so the
draw count cannot fall) plus the split of `rec` into `recbuild`/`recenc`.

**What changed.**

- `rec` is reported as `recbuild` (frustum setup, culling, material
  resolution, sort-key finalization, sort) and `recenc` (instance-matrix pass,
  state binds, draw emission); the remainder is the command-buffer/render-pass
  prologue and `RessourceManager::sync()`.
- Materials are resolved once into a cache indexed by material ID (flat for
  the dense provider range, a small map for variant IDs) and invalidated by a
  new `RessourceManager::getMaterialGeneration()` counter, so a draw no longer
  costs four hash/map lookups.
- The world-space AABB, its centre and its radius are transformed once per
  object per frame (`Renderer::drawFrame`) instead of once per object per
  view.
- The sort orders a `uint32_t` index array over a parallel key array and
  permutes the draw list once, instead of moving the items (each carrying a
  `glm::mat4`) through every comparison and swap. The run length of each draw
  is resolved during that permutation, so the encode loop jumps from run head
  to run head.
- The instance matrices are written in one contiguous pass over the recording
  order, and the per-item loop no longer re-reads the instance buffer handle,
  the dynamic offset or the pipeline layout.

**Method.** Same machine and configuration as §1 (Debug, validation on,
800×600, single view, `EVAN_STATS_INTERVAL=60`, 5 kept windows). The two
builds were measured back to back in one session; the host was not idle.
`EVAN_POC_UNIQUE_MESHES=1` is new, so the "before" column is the previous
renderer running today's workload.

### 9.1 `EVAN_POC_UNIQUE_MESHES=1`, N=1000 (1000 draws/frame)

| build | `rec` (ms) | `recbuild` (ms) | `recenc` (ms) | `rec`/draw | `pso`/`ds` | `draws` |
| --- | --- | --- | --- | --- | --- | --- |
| before | 6.28–6.41 | – | – | 6.3–6.4 µs | 1 / 1 | 1000 |
| after | 2.25–2.32 | 0.56 | 1.57–1.62 | 2.3 µs | 1 / 1 | 1000 |

### 9.2 Mergeable workload, N=1000 (1 draw/frame)

| build | `rec` (ms) | `recbuild` (ms) | `recenc` (ms) | `draws` | `inst` | `pso`/`ds` |
| --- | --- | --- | --- | --- | --- | --- |
| before | 4.75–5.10 | – | – | 1 | 1 | 1 / 1 |
| after | 0.61 | 0.58 | 0.01 | 1 | 1 | 1 / 1 |

The tables above are the last of several runs; every repeat of the unique
workload stayed within 2.25–2.88 ms (2.3–2.9 µs per draw) and every repeat of
the mergeable one within 0.61–0.80 ms, so the spread is host load, not a
regression between runs.

### 9.3 Acceptance

| Criterion | Result |
| --- | --- |
| unique N=1000 `rec`/draw ≤ 3.0 µs (from 5.36) | 2.3 µs (2.3–2.9 across runs) |
| unique N=1000 `rec` ≤ 3.5 ms | 2.25–2.32 ms |
| unique N=1000 `recbuild` ≤ 25 % of `rec` | 24.6 % (20.5–24.6 % across runs) |
| mergeable N=1000 `draws` ≤ 2, `rec` ≤ 1.0 ms | 1 draw, 0.61 ms |
| draw order unchanged | the sorted list (sequence, blend mode, pipeline, material, vertex/index buffer, depth bucket) is identical to the previous renderer for 40 shared-mesh and 40 unique-mesh objects |
| culling counters unchanged | `vis` = 1000 and `culled` = 0 per frame on both workloads, same as before |
| `ctest` green | 388/388 |

The remaining per-draw cost is almost entirely `recenc` (1.57–1.62 ms for
1000 draws, i.e. ~1.6 µs per draw at `-O0` with validation active). It is now
the dominant term, which is consistent with the validation-layer overhead
noted in §5: the Release/validation-off control is the next measurement, not
another change to the shared per-item code. §10 records it.

## 10. Post-optimization record and control (2026-11-19)

§3 is the 2026-10-29 baseline on the per-object-`GPUMesh` build, §8 adds the
geometry cache and the instanced merge (#175) and §9 adds the draw-list build
and per-draw recording work (#176). None of them records the engine *after*
the milestone, and §9.3 leaves the Release/validation-off control as the next
measurement. This section closes both gaps: the same harness on the build
after #175–#177, against the pre-optimization build in the same session, with
the control next to the reference.

**Environment.** Apple M3 (macOS 27.2.1, build 26B131, Darwin 27.2.0, arm64),
MoltenVK from Vulkan SDK 1.4.304.1, 800×600, default MSAA, single view,
`EVAN_STATS=1 EVAN_STATS_INTERVAL=60`, `UTILITY_LOG_LEVEL=info`,
`NO_COLOR=1`, `scripts/run-poc.sh` defaults (420 frames per N, 2 warm-up
windows, 5 kept windows). Host load average 3.2–7.7 during the session — the
host was **not** idle, see §10.5.

**Configurations.**

| Label | Build | Validation | Purpose |
| ----- | ----- | ---------- | ------- |
| before | `82ee737` (main before #175), `CMAKE_BUILD_TYPE=Debug` | `VK_LAYER_KHRONOS_validation` on | the pre-optimization baseline |
| reference | current tree, `CMAKE_BUILD_TYPE=Debug` | on | comparable with §1–§9 |
| control | current tree, `CMAKE_BUILD_TYPE=Release` | off | the engine's own cost |

The validation layer is selected by `NDEBUG` (`EvanPlatform.hpp`), and the
Release build defines it, so the control build has no validation layer, no
debug messenger and no `-O0`.

**Method.** Five runs per configuration, interleaved as
`before, reference/shared, reference/unique, control/shared, control/unique`
per round, so host load hits every side of every comparison equally. Each
table below quotes the **median of the five runs**; §10.3 has the per-run
spread. The pre-#175 build has no `EVAN_POC_UNIQUE_MESHES` knob and does not
need one: it uploads one `GPUMesh` per object even for identical content, so
its default run already *is* the per-draw workload (1000 draws for 1000
meshes, `inst=0`). It also predates the `recbuild`/`recenc` split, so those
cells are `–`.

### 10.1 Reference configuration (Debug + validation)

Shared-geometry workload (the default, #175):

| N | build | `rec` (ms) | `rec`/mesh (µs) | `recbuild` (ms) | `recenc` (ms) | `draws` | `inst` | `pso` | `ds` | start `mesh_churn` | steady `mesh_churn` | `uploadwaits` | `culled` | `pass0` (ms) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 100 | before | 1.62 | 16.2 | – | – | 100 | 0 | 1 | 1 | +100/-0 | +0/-0 | 0 | 0 | 0.13 |
| 100 | after | 0.25 | 2.5 | 0.14 | 0.04 | 1 | 1 | 1 | 1 | +1/-0 | +0/-0 | 0 | 0 | 0.08 |
| 1000 | before | 5.95 | 5.95 | – | – | 1000 | 0 | 1 | 1 | +1000/-0 | +0/-0 | 0 | 0 | 0.23 |
| 1000 | after | 0.76 | 0.76 | 0.70 | 0.02 | 1 | 1 | 1 | 1 | +1/-0 | +0/-0 | 0 | 0 | 0.09 |

Distinct-geometry workload (`EVAN_POC_UNIQUE_MESHES=1`, #176):

| N | build | `rec` (ms) | `rec`/draw (µs) | `recbuild` (ms) | `recenc` (ms) | `draws` | `inst` | `pso` | `ds` | start `mesh_churn` | steady `mesh_churn` | `uploadwaits` | `culled` | `pass0` (ms) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 100 | before | 1.62 | 16.2 | – | – | 100 | 0 | 1 | 1 | +100/-0 | +0/-0 | 0 | 0 | 0.13 |
| 100 | after | 0.71 | 7.1 | 0.16 | 0.46 | 100 | 0 | 1 | 1 | +100/-0 | +0/-0 | 0 | 0 | 0.13 |
| 1000 | before | 5.95 | 5.95 | – | – | 1000 | 0 | 1 | 1 | +1000/-0 | +0/-0 | 0 | 0 | 0.23 |
| 1000 | after | 2.50 | 2.50 | 0.59 | 1.72 | 1000 | 0 | 1 | 1 | +1000/-0 | +0/-0 | 0 | 0 | 0.26 |

`mesh_churn` is the startup window (`uploadwaits=1`) and the steady state;
`uploadwaits` the steady state. `pso`/`ds` is one bind per draw *run*: the
engine registers a debug-ray object when the pointer enters the window, which
adds a second run and makes those counters read 2 (and `draws` N→N+1) in that
run. It registered in four of the fifty workload runs, all in the session's
first round; the medians do not show it.

Paired per-round ratios (before ÷ after, same workload and round): 7.7–9.4×
at N=1000 and 3.9–6.7× at N=100 on the shared workload, 2.2–2.4× and
1.7–2.3× on the distinct-geometry one. The N=100 shared low end comes from
the one pre-optimization run that reproduced the idle-host value of §3
(1.00 ms), the rest of its spread is host load.

### 10.2 Control (Release, validation off)

| Workload | N | `rec` (ms) | `rec`/draw (µs) | `recbuild` (ms) | `recenc` (ms) | `draws` | `inst` | `pso`/`ds` | `pass0` (ms) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| shared | 100 | 0.048 | 0.48 per mesh | 0.022 | 0.010 | 1 | 1 | 1 / 1 | 0.11 |
| shared | 1000 | 0.200 | 0.20 per mesh | 0.172 | 0.012 | 1 | 1 | 1 / 1 | 0.11 |
| unique | 100 | 0.116 | 1.16 | 0.028 | 0.064 | 100 | 0 | 1 / 1 | 0.19 |
| unique | 1000 | 0.492 | 0.49 | 0.170 | 0.284 | 1000 | 0 | 1 / 1 | 0.74 |

What the control separates:

- **The remaining `rec` is mostly not the engine.** At N=1000 with 1000
  distinct meshes the reference measures 2.50 ms and the control 0.49 ms, a
  factor of 5.1 (4.9–5.2 in the last four rounds). The 2.0 ms difference is
  the `-O0` build plus validation-layer interception of every recorded
  command — the recorder's own cost is ~0.5 µs per draw.
- **The optimizations still hold without validation.** The control keeps the
  same shape: merged scene 0.20 ms / 1 draw, 1000 distinct meshes 0.49 ms /
  1000 draws, `recbuild` ≈ 0.17 ms in both (the draw-list build is per mesh,
  so it does not collapse with the merge).
- **`pass0` is not comparable across the two columns.** The control ran last
  in each round and its 1000-draw `pass0` (0.53–0.78 ms) is higher than the
  reference's (0.24–0.47 ms) for identical GPU work; the timer is coarse
  (§7). Read `pass0` inside one configuration only.

**Configuration future numbers must use.** Record new numbers on the
**control build (Release, validation off)** — that is the engine's cost, and
it is the only number that answers "did this change the engine". Keep the
Debug/validation reference in the same session as a regression baseline
against §1–§9, because the two differ by ~5× on `rec`. A number quoted without
its configuration is meaningless from here on.

### 10.3 Spread across runs

Five runs per configuration, 5 steady-state windows each:

| Configuration | N | `rec` (ms) | `recbuild` (ms) | `recenc` (ms) | `pass0` (ms) |
| --- | --- | --- | --- | --- | --- |
| before | 100 | 1.00–1.67 | – | – | 0.11–0.26 |
| before | 1000 | 5.92–6.17 | – | – | 0.21–0.44 |
| reference, shared | 100 | 0.23–0.30 | 0.14–0.17 | 0.04–0.05 | 0.08–0.22 |
| reference, shared | 1000 | 0.66–0.77 | 0.59–0.73 | 0.02–0.03 | 0.08–0.25 |
| reference, unique | 100 | 0.61–0.75 | 0.12–0.16 | 0.38–0.47 | 0.11–0.23 |
| reference, unique | 1000 | 2.45–2.86 | 0.58–0.60 | 1.70–2.01 | 0.24–0.47 |
| control, shared | 100 | 0.04–0.06 | 0.02 | 0.01 | 0.08–0.23 |
| control, shared | 1000 | 0.09–0.24 | 0.08–0.20 | 0.00–0.02 | 0.09–0.23 |
| control, unique | 100 | 0.07–0.14 | 0.02–0.03 | 0.04–0.08 | 0.16–0.29 |
| control, unique | 1000 | 0.30–0.52 | 0.09–0.18 | 0.17–0.30 | 0.53–0.78 |

`recbuild` in the reference unique workload is the stable one (0.58–0.60 ms
at N=1000): it is per-mesh work, and 1000 meshes cost the same every run. The
control is a ~0.2–0.5 ms quantity, so a single timer/load step is a large
share of its range; its N=1000 values drifted upward across the session
(0.30 → 0.52 ms) while the reference did not (2.45–2.86 ms), which is the
expected shape of background load on a small number.

`draws`, `inst`, `pso`, `ds`, `mesh_churn`, `uploadwaits` and `culled` are
stable across all 25 runs: 1 draw / `inst=1` for the merged scene and one
draw per mesh for the distinct-geometry one, `pso`/`ds` one bind per run
(2 only in the runs with the debug ray), startup churn as listed in §10.1,
steady churn `+0/-0`, `uploadwaits` 0, `culled` 0 — except one shared N=1000
run that reported `vis` 905 / `culled` 96 per frame throughout, i.e. the view
in that run covered the grid differently; it is the same run that registered
the ray, and its `recbuild` (0.59 ms) is the lowest of the five.

### 10.4 Reading

**Milestone exit criteria** (issue acceptance lists):

| Criterion | Target | Measured | Verdict |
| --- | --- | --- | --- |
| #175 N=1000 `draws` | ≤ 2 | 1 (2 with the debug ray) | met |
| #175 N=1000 `instancedDraws` | ≥ 1 | 1 per frame | met |
| #175 N=1000 `rec` | ≤ 1.0 ms | 0.76 reference, 0.20 control | met |
| #175 `pso`, `ds` | 1 / 1 | 1 / 1 per run | met |
| #175 startup `mesh_churn`, steady, `uploadwaits` | +1/0, +0/-0, 0 | +1/-0, +0/-0, 0 | met |
| #175 image, shader gate | identical, never merge without instance inputs | §8.3 | met |
| #176 unique N=1000 `rec`/draw | ≤ 3.0 µs | 2.50 reference, 0.49 control | met |
| #176 unique N=1000 `rec` | ≤ 3.5 ms | 2.50 | met |
| #176 `recbuild` share | ≤ 25 % of `rec` | 23.4 % (0.586 / 2.500) | met |
| #176 mergeable `draws`, `rec` | ≤ 2, ≤ 1.0 ms | 1, 0.76 | met |
| #176 draw order, culling, image; phases parsed | unchanged; `run-poc.sh` parses them | §9.3; harness | met |
| #177 N=1000 `pass0` | ≤ 0.45 ms | 0.09 merged, 0.26 distinct (median); 0.47 worst run | met at the median, one run one timer step over |
| #177 closed meshes, `None` override, no VUID | identical; override works; clean | §7, §9; 25 runs, no validation output, `rc=0` | met |
| `ctest` | green | 390/390 | met |

**The milestone's per-draw target is met.** At N=1000 with 1000 distinct
meshes the recorder costs 2.50 µs per draw in the reference configuration and
0.49 µs in the control; #176 targeted 3.0 µs from a measured 5.36 µs, and the
pre-optimization build re-measures at 5.95 µs in the same session.

**#179's trigger did not fire.** It asks for a distinct-geometry N=1000 run
that *still* spends more than 3.0 µs of `rec` per draw, or a 50 % recording
budget at the target refresh below the scene's draw count, *with* `recenc`
dominating. `recenc` does dominate (1.72 of 2.50 ms in the reference, 0.28 of
0.49 ms in the control — 58–69 %), but the absolute numbers are inside the
target: 2.50 µs/draw in the reference and 0.49 µs in the control, and 50 % of
a 120 Hz frame (4.17 ms) holds 1667 draws in the reference configuration and
≈8500 in the control, both above the 1000 draws of the workload. GPU-driven
multi-draw stays a design note.

### 10.5 Caveats

- **Host load.** The session ran at load average 3.2–7.7, not the <1 of the
  2026-10-29 record, so absolute `rec` values are inflated. The
  pre-optimization build re-measures at 5.92–6.17 ms for N=1000 against
  5.80 ms in §3, and at 1.00–1.67 ms for N=100 against 0.98 ms — the low end
  reproduces the idle record, the rest is load. Every before/after and
  reference/control comparison above is therefore *within* one round, and the
  ratios are what transfer; re-recording on an idle host should move the
  absolute numbers down, not reorder them.
- **Validation cost is not isolated from `-O0`.** `enableValidationLayers` is
  `#ifdef NDEBUG`, so the control is Release *and* validation-off. A ~5×
  factor is the sum of both. Separating them needs a Release build with
  `NDEBUG` undefined and the layer force-enabled
  (`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`); that is a build-system
  change and is not done here.
- **GPU timer.** `pass0` moves in ~0.1 ms steps and the cubes are ~13 px at
  N=1000; treat it as an order-of-magnitude signal within one configuration,
  as in §7.
- **Two objects outside the workload.** The debug-ray object (first
  mouse-motion event) adds one non-mergeable draw, one pipeline bind and one
  descriptor bind in the runs where it registers, which is why `draws` reads
  N+1 and `pso`/`ds` read 2 there.
- **Still missing for WS4.** A per-view `rec` split on the OpenXR stereo
  build. §10.2 now bounds the engine-side recording cost well enough to keep
  serial recording deferred (§4 of the design), but the per-view number is
  still the only input that closes the trigger.
