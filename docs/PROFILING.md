# Profiling evan

This page is the measurement runbook referenced by
[`DESIGN_INSTRUMENTATION.md`](DESIGN_INSTRUMENTATION.md): how to use the
built-in frame statistics, how to confirm findings with external GPU tools,
and how to reproduce the N=100 / N=1000 proof-of-concept runs.

## 1. Built-in frame statistics (`EVAN_STATS`)

Off by default. Everything below costs a single predictable branch per hook
when disabled.

| Variable | Default | Meaning |
| -------- | ------- | ------- |
| `EVAN_STATS` | off | Master switch: CPU phase timers, counters, GPU timestamps and the interval log. Any non-empty, non-`0` value enables it. |
| `EVAN_STATS_INTERVAL` | `120` | Frames per report window (one aggregated log record per window). |
| `EVAN_STATS_GPU` | on | `0` disables timestamp queries even when supported. |

```sh
EVAN_STATS=1 EVAN_STATS_INTERVAL=60 ./example
```

Every 60 frames one line is logged (nothing is logged per frame, never per
mesh):

```
evan stats: frames=120 | cpu_ms(avg): flush=0.01 pre=0.02 acq=0.05 wait=1.02 ubo=0.01 rec=0.84 submit=0.03 renderwait=-- pres=0.04 post=0.00 | gpu_ms(avg): pass0=3.21 pass1=3.19 | meshes: total=1000 vis=998 culled=2 skip=0 | draws: calls=998 inst=420 pso=12 ds=12 | sync: calls=240 skip=238 | mesh_churn: +12/-12 | uploadwaits=1
```

Reading the line:

- `cpu_ms(avg)` — per-frame CPU phase averages:
  - `flush`: submitting pending uploads; `pre`: OpenXR preprocess
    (`xrWaitFrame` pacing); `acq`: swapchain image acquire;
    `wait`: waiting for the frame slot's in-flight fence (GPU back-pressure);
    `ubo`/`rec`/`submit`: per-view uniform update, recording and queue submit;
    `renderwait`: OpenXR end-of-frame wait; `pres`: present/`xrEndFrame`;
    `post`: OpenXR postprocess.
- `gpu_ms(avg)` — per-pass GPU time from timestamp queries (`pass0`/`pass1`
  are view slots; in multiview the single pass covers both eyes). `--` means
  the device/report has no sample; with stats on, the first frames after a
  resize legitimately show `--` (queries read one slot-reuse later).
- `meshes`/`draws`/`sync`/`mesh_churn`/`uploadwaits` — per-frame counters:
  culling efficiency, draw-call and bind counts, `RessourceManager::sync()`
  calls vs skipped, `GPUMesh` creation/destruction churn, and blocking upload
  fence waits (the `vkQueueWaitIdle`-per-upload signal).
- `--` in any CPU phase means the backend has no sample for it (e.g.
  `renderwait` on GLFW).

How to read a frame:

- **CPU-bound**: `rec` (plus `sync` counters) dominates and `wait` is small —
  the renderer cannot record draws fast enough; look at the draw counters
  (`calls`, `pso`, `ds`) and the audit's P0/P1 CPU findings.
- **GPU-bound**: `gpu_ms` ≈ the frame budget while CPU phases are small —
  confirm with the GPU tools below (fill rate, bandwidth, draw-call count).
- **Serialized**: large `wait` + `renderwait` while `rec` is small — the
  CPU is idle waiting for the GPU; revisit synchronization findings (P0-03 /
  P1.5).
- Compare configurations without rebuilding: `EVAN_MULTIVIEW=off`,
  `EVAN_MSAA=1|2|4`, `EVAN_DEBUG_DISABLE_CULLING=1`,
  `EVAN_DEBUG_DRAW_LOG=1` (per-mesh trace, keep off for measurements).

The same data is available programmatically:
`renderer.getStats()` / `engine.getFrameStats()` return the current window
(phases in ns via `PhaseStats`, counters) for a host-side debug overlay.

## 2. RenderDoc

Best for draw-call/state inspection and per-draw GPU timing.

1. Launch the app under RenderDoc (or set `ENABLE_VULKAN_RENDERDOC_CAPTURE=1`
   and press the capture hotkey in-app; on macOS capture the GLFW build).
2. Capture one frame.
3. Cross-check the event browser against the stats line: number of
   `vkCmdDrawIndexed`/`vkCmdDrawIndexedIndirect` calls (`draws.calls`),
   pipeline binds (`pso`), descriptor binds (`ds`), and the mesh count.
4. Use the GPU timing column for per-draw attribution; if a single mesh is
   the hotspot, the per-pass `gpu_ms` plus the event list identifies it.

## 3. Nsight Graphics (NVIDIA)

1. Start the app under Nsight Graphics (Vulkan capture).
2. Use the frame debugger for state, the GPU Trace for per-pass timing.
3. Confirm the `gpu_ms` breakdown (pass0/pass1) against the GPU Trace ranges,
   and check SM occupancy/bandwidth if a pass is GPU-bound.

## 4. Radeon GPU Profiler (AMD)

1. Enable the RGP layer/remote session, fetch a profile of a few frames.
2. Compare per-pass durations with the stats log; use the pipeline state and
   event timing views for per-draw attribution on RDNA hardware.

## 5. Validation and performance layers

Always capture with the validation layers active when investigating:

```sh
VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation ./example
```

- **`VK_LAYER_KHRONOS_validation`**: API misuse, synchronization validation
  (`VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT`) and
  GPU-assisted validation. The debug build enables validation by default.
- **Profiles layer** (`VK_LAYER_KHRONOS_profiles`): pin device capabilities
  (e.g. force `EVAN_MULTIVIEW`/`EVAN_MSAA` support on or off) to compare code
  paths deterministically.
- Vendor performance layers where available (e.g. `VK_LAYER_AMD_*`).

With stats enabled, a validation run must stay clean: the timestamp pool is
created only when `EVAN_STATS`/`EVAN_STATS_GPU` allow it, and queries are
read with `VK_QUERY_RESULT_WITH_AVAILABILITY_BIT` (never blocking).

## 6. PoC recipe — N=100 vs N=1000

The `GLFW-macos-poc` example ships the workload and `scripts/run-poc.sh`
drives it, so the whole run is one command:

```sh
scripts/run-poc.sh            # N=100 and N=1000, 7 windows each
```

It builds the example (which also copies `shaders/` and `textures/` next to
the binary), runs each N with `EVAN_STATS=1 EVAN_STATS_INTERVAL=60`, discards
the warm-up windows, averages the rest and writes `summary.md` plus the raw
logs under `build/poc-results/<timestamp>/`. `--mesh-counts`, `--interval`,
`--warmup`, `--windows`, `--frames` and `--out` override the defaults.

Run it by hand with the same knobs:

| Variable | Default | Meaning |
| -------- | ------- | ------- |
| `EVAN_POC_MESHES` | `0` | Cubes to spawn: one shared local-space mesh, one `Primitive` per object, placed on a grid in front of the camera through `Engine::setObjectTransform`. `0` keeps the empty scene. |
| `EVAN_POC_FRAMES` | `0` | Stop after that many frames and exit (`0` runs until the window is closed). Needed for a reproducible run that flushes its log. |

```sh
cd build/examples/GLFW-macos-poc
EVAN_STATS=1 EVAN_STATS_INTERVAL=60 \
  EVAN_POC_MESHES=1000 EVAN_POC_FRAMES=420 \
  UTILITY_LOG_LEVEL=info NO_COLOR=1 ./GLFW-macos-poc
```

`UTILITY_LOG_LEVEL=info` matters in release builds (which suppress `info`)
and in debug builds (which otherwise log every vertex); `NO_COLOR=1` keeps the
parsed values free of ANSI escapes. Keep the window size (800×600) and MSAA
the same for every N.

Counters in a `evan stats:` line are **sums over the window**, not per-frame
values (`cpu_ms`/`gpu_ms` are already per-frame averages), so divide them by
`frames=` before comparing; `run-poc.sh` does that.

One measurement note: the engine registers a debug ray object on the first
mouse-motion event even though this sample sets no event callback, so `draws
calls` reads N or N+1 depending on whether the pointer entered the window.

Fill the table (averages over the steady-state windows, churn-free ones):

| Metric | N=100 | N=1000 |
| ------ | ----- | ------ |
| `rec` (ms) | | |
| `wait` (ms) | | |
| `gpu_ms` pass0 / pass1 | | |
| `draws.calls` / `inst` | | |
| `pso` / `ds` binds | | |
| `sync` calls / skip | | |
| `mesh_churn` | | |
| `uploadwaits` | | |
| measured FPS | | |

Variations worth running through the same harness (exported variables are
inherited by the example):

- `EVAN_MSAA=4` vs `EVAN_MSAA=1` — bandwidth cost of multisampling.
- `EVAN_DEBUG_DISABLE_CULLING=1` — culling effect at N=1000.
- `--mesh-counts "100 1000 5000"` — draw-call scaling on the same machine.
- The OpenXR stereo build — the only configuration that gives WS4 its
  per-view `rec` signal.

Record the results in `docs/DESIGN_P3_OPTIMIZATIONS.md` §4 (“PoC results”) and
under “Expected impact” in
[`PERFORMANCE_AUDIT_EVAN.md`](PERFORMANCE_AUDIT_EVAN.md).
