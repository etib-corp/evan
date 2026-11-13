# P3 proof-of-concept results — N=100 vs N=1000

Status: recorded run on the fixed build (desktop single view, macOS / MoltenVK). This file
is the measurement record the WS4/WS5/WS6 gates of
[`DESIGN_P3_OPTIMIZATIONS.md`](DESIGN_P3_OPTIMIZATIONS.md) §4 are decided on.
It is the expanded `summary.md` produced by the harness, kept next to the
design so the raw windows stay reviewable.

Date: 2026-10-29 (numbers re-recorded after the shutdown/validation fixes
described in §7; the first record of the same day was taken on the pre-fix
build and is superseded)
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
- **Culling.** Back-face culling is currently disabled in the static pipeline
  state (it used to be declared as a dynamic state that was never set, i.e.
  undefined, and it was also a spec violation on every pipeline). Both faces
  are therefore rasterized, which is why `pass0` is a double-sided fill cost.
  Re-enabling culling needs a front-face decision first, because the
  projection flips Y for Vulkan.
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
