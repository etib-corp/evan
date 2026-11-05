# Stress testing the renderer — finding the engine limits

Status: implemented (issue #173). Harness: [`scripts/run-stress.sh`](../scripts/run-stress.sh),
workload: [`examples/GLFW-stress`](../examples/GLFW-stress).

This page explains how to find *how far the engine goes*: how many meshes and
how many distinct pipelines it can hold at a given frame-rate threshold, and
what the renderer's own indicators say about the bottleneck at that boundary.

The runbook for the built-in counters and for external GPU tools is
[`PROFILING.md`](PROFILING.md); this page is the sweep on top of it.

## 1. What is measured

Every probe renders one configuration and reports:

| Dimension | Knob | Meaning |
| --------- | ---- | ------- |
| Mesh count | `--meshes N` | Number of registered objects. Each object owns its `GPUMesh` (vertex + index buffer) and produces one draw. |
| Pipeline count | `--pipelines K` | Number of distinct graphics pipelines the scene draws through. The objects cycle over K materials, each bound to its own shader module. |
| Frame rate | `fps`, `fps_p50` | Mean frame rate of the measured frames, and the frame rate of the median frame (the decision metric of the sweep). |
| Frame-time spread | `frame_ms_p50/p95/p99/max` | Nearest-rank percentiles of the same frames; `fps_1pct_low` is `1000 / max`. |
| Cost split | `rec_ms`, `submit_ms`, `wait_ms`, `ubo_ms`, `pres_ms`, `gpu_ms` | Renderer CPU phases and GPU pass time, per frame, from `FrameStats`. |
| Draw behaviour | `draws`, `instanced`, `pso_binds`, `ds_binds` | Per frame. `pso`/`ds` are the pipeline and descriptor-set binds: the cost of state switching. |
| Culling | `vis`, `culled`, `skip` | Per frame. `vis == meshes` means the whole scene reached the draw list. |
| Startup | `init_ms`, `scene_ms` | Engine construction (device, pipelines, frames) and scene upload (buffers, materials, textures). |

The thresholds solved by the script are 120, 100, 75, 60, 30 and 24 fps. For
each one it reports the largest mesh count (at a fixed pipeline count) and the
largest pipeline count (at a fixed mesh count) that still holds the threshold.

## 2. Running it

```sh
# Full sweep with the default budget (240 measured frames per probe).
scripts/run-stress.sh

# Fast smoke run: small bounds, 90 frames per probe.
scripts/run-stress.sh --quick

# One threshold, custom bounds and pipeline workload.
scripts/run-stress.sh --thresholds 60 --meshes-max 32768 \
  --pipelines-max 32 --base-meshes 2000

# Compare two optimization configurations.
scripts/run-stress.sh --out build/stress-results/culling-on  --culling on
scripts/run-stress.sh --out build/stress-results/culling-off --culling off
```

The script builds the example (configure the build tree with
`-DBUILD_EXAMPLES=ON`), runs the probes, and writes:

| File | Content |
| ---- | ------- |
| `summary.md` | Limit tables and the indicators at every boundary configuration. |
| `runs.tsv` | Every probed configuration, one row per `(meshes, pipelines)`. |
| `results.csv` | The same runs as CSV, written by the example itself. |
| `probe-m<N>-p<K>.log` | Full log of one probe (validation messages, warnings). |
| `environment.txt` | Machine, OS, GPU, window size and probe budget. |

Output directory default: `build/stress-results/<UTC timestamp>/`.

### The example directly

```sh
cd build/examples/GLFW-stress
NO_COLOR=1 UTILITY_LOG_LEVEL=warning ./GLFW-stress \
  --meshes 2000 --pipelines 4 --frames 240 --warmup 60 \
  --present-mode mailbox --csv /tmp/stress.csv
```

It prints one record:

```text
evan stress: meshes=2000 created=2000 pipelines=4 frames=240 warmup=60
fps=76.20 fps_1pct_low=... frame_ms_p50=... frame_ms_p95=... | stat_frames=240
vis=2000 culled=0 skip=0 draws=2000 instanced=0 pso_binds=111 ds_binds=111
... rec_ms=12.28 submit_ms=... gpu_ms=...
```

(The real record is a single line; it is wrapped here for readability.)

Knobs can also be set through the environment:
`EVAN_STRESS_MESHES`, `EVAN_STRESS_PIPELINES`, `EVAN_STRESS_FRAMES`,
`EVAN_STRESS_WARMUP`, `EVAN_STRESS_INSTANCING`, `EVAN_STRESS_INDIRECT`,
`EVAN_STRESS_CULLING`. The example publishes `EVAN_STATS=1`,
`EVAN_STATS_INTERVAL=<warmup + frames + 1>` and, when requested,
`EVAN_PRESENT_MODE` and `EVAN_DEBUG_DISABLE_CULLING` to the engine before
anything is created, so the counters and the present mode apply to the whole
run. An explicit user value always wins.

## 3. Method

- **One process per configuration.** Each probe creates a window, a device,
  every pipeline and the whole scene, so `init_ms`, `scene_ms` and the frame
  budget are measured for that configuration only. Process start-up dominates
  a probe at high frame rates; the search accounts for it by using the same
  budget everywhere.
- **Warm-up, then reset.** `--warmup` frames run before measurement; the
  statistics window is reset right after them, so uploads, pipeline creation
  and driver warm-up never reach the reported phases.
- **Search.** Exponential expansion (`1, 2, 4, ...` up to the bound) finds a
  passing/failing bracket, then `--refine` binary steps narrow it to ~2 % of
  the limit (or one unit, for small limits). Every probed `(meshes, pipelines)`
  pair is cached in `runs.tsv`, so thresholds that pass through the same point
  do not measure it twice. `--max-probes` caps the whole run.
- **Percentile-based decisions.** The threshold test uses the frame rate of
  the *median* frame (`fps_p50 = 1000 / p50`). The mean (`fps`), `p95`/`p99`
  and `fps_1pct_low` are reported next to it: a single driver-side stall (the
  first use of a freshly created pipeline can take hundreds of milliseconds)
  would otherwise decide a boundary on its own, and a scene can hold a mean
  while stuttering.
- **Shaders for the pipeline axis.** All pipelines must exist before the
  `Engine` is constructed (the `Renderer` creates one pipeline per registered
  shader at construction). The harness copies `mesh.vert.spv`/`mesh.frag.spv`
  into a scratch directory under distinct names, registers them, and binds
  each object to one variant through `Engine::addMesh(mesh, material, shader)`
  (material variants share the material's textures). The SPIR-V is identical,
  so the axis measures pipeline/state cost, not shader complexity.

## 4. Reading the results

### Limits

The first table answers "how much scene can this machine push at T fps". A
limit equal to the search bound means the bound was hit, not the engine: raise
`--meshes-max`/`--pipelines-max` and rerun. A limit of `0` means the threshold
was unreachable even with one mesh — check `init_ms` and the window/refresh
rate before blaming the scene.

### Indicators

| Indicator | What it tells you |
| --------- | ----------------- |
| `rec_ms` growing with `--meshes` | CPU draw-list and command recording is the bottleneck. This is the dominant cost in the current renderer. |
| `gpu_ms` growing with mesh count or resolution | GPU-bound: the pass time approaches the frame budget while `rec_ms` stays flat. |
| `pso`/`ds` binds | State switching. They follow the pipeline count, and with the default front-to-back sort they also grow with the number of depth buckets, because draw groups get split across pipelines. |
| `draws` vs `instanced` | Batching. Merging currently requires consecutive draws to share pipeline, material *and* vertex/index buffers; the harness gives every object its own `GPUMesh`, so `instanced` stays `0` and `--instancing on` measures the batching path's overhead rather than a reduction. A workload that shares geometry (the issue this mirrors) is what makes the counter drop. |
| `vis`/`culled` | `culled` stays `0` while the grid fits the view; use `--culling off` to measure the culling gain at the same scene. |
| `submit_ms`, `wait_ms`, `pres_ms` | Queue and presentation cost. Under vsync `pres_ms`/`submit_ms` absorb the pacing and the frame rate plateaus at the refresh rate. |
| `init_ms` | Pipeline creation and device setup. It scales with `--pipelines`, and the first run on a machine is much slower (driver shader/pipeline caches are cold) — do not compare the first probe of a session with later ones. |
| `scene_ms` | Buffer, material and texture upload for the whole scene. |

### Display pacing (important)

`fps` is wall-clock frame rate. When presentation is paced by the display —
FIFO by definition, and on macOS/MoltenVK in every present mode, because
`CAMetalLayer` is driven by the display link — no scene can exceed the refresh
rate. On a 120 Hz panel a 100-mesh scene and a 500-mesh scene both report
~120 fps; the difference is only visible in `rec_ms`/`gpu_ms`. Thresholds
above the refresh rate are then unreachable by construction.

Use `--present-mode mailbox` (the script default) on Linux and Windows to
measure the throughput ceiling above the refresh rate, and read `rec_ms` +
`gpu_ms` against the frame budget when the rate plateaus.

## 5. Recorded runs

Two runs are recorded: a macOS laptop whose display paces the frame rate, and
a Linux desktop measured in `mailbox`, above the refresh rate. Numbers are
illustrative and machine-specific; compare runs only on the same machine and
in the same present mode.

### macOS 27.2 / MoltenVK (Apple M3)

Machine: Apple M3, macOS 27.2, MoltenVK (Metal 4), 800×600, single view,
`EVAN_MSAA=1`, debug build with validation, culling on, instancing off,
indirect off. The built-in panel is 120 Hz, so 120 fps is the measurable
ceiling on this machine.

Mesh sweep at one pipeline (`--frames 120 --warmup 30`; `fps` is the mean
frame rate of the measured frames):

| meshes | fps | p95 (ms) | rec (ms) | gpu (ms) |
| --- | --- | --- | --- | --- |
| 500 | 120.03 | 8.95 | 3.40 | 0.11 |
| 2000 | 78.55 | 13.17 | 11.90 | 0.35 |
| 8000 | 17.51 | 59.78 | 53.91 | 2.80 |

Pipeline sweep at 2000 meshes:

| pipelines | fps | pso binds/frame | ds binds/frame | rec (ms) |
| --- | --- | --- | --- | --- |
| 1 | 77.57 | 1 | 1 | 12.05 |
| 4 | 76.20 | 111 | 111 | 12.28 |
| 16 | 71.93 | 426 | 426 | 12.97 |
| 32 | 67.17 | 771 | 771 | 13.82 |

Boundary search on the same machine (`--thresholds 60 --frames 120 --warmup
30`): the 60 fps mesh limit at one pipeline was **2432 meshes** (60.8 fps), and
the 60 fps pipeline limit at 2000 meshes was **above 32** (67.2 fps), i.e. the
search bound, with 771 pipeline/descriptor binds per frame.

Interpretation: recording dominates (`rec_ms` follows the mesh count while
`gpu_ms` stays under 3 ms), and pipeline count mostly buys state switching
(771 binds per frame at 32 pipelines with the front-to-back sort) rather than
GPU work. Both are the signals a limit sweep is supposed to expose.

### Linux x86_64 desktop (`mailbox`)

Full sweep with the script defaults (240 measured frames after 60 warm-up
frames per probe, culling on, instancing off, indirect off, search bound
`131072` meshes / `32` pipelines), 800×600, single view, present mode
`mailbox`. Host: `Linux 7.1.5-76070105-generic / x86_64`; recorded
2026-11-05. Nothing paces the frame rate here, so these are throughput limits
and sit far above any refresh rate.

| Threshold | Max meshes (1 pipeline) | FPS at limit (p50) | Max pipelines (1000 meshes) | FPS at limit (p50) |
| --- | --- | --- | --- | --- |
| 120 fps | 14848 | 123.73 | 32 | 1365.52 |
| 100 fps | 17408 | 100.20 | 32 | 1365.52 |
| 75 fps | 22528 | 75.76 | 32 | 1365.52 |
| 60 fps | 25600 | 67.68 | 32 | 1365.52 |
| 30 fps | 38912 | 37.76 | 32 | 1365.52 |
| 24 fps | 47104 | 28.42 | 32 | 1365.52 |

Indicators at the mesh limit:

| Threshold | meshes | FPS | p50 (ms) | p95 (ms) | draws | pso | ds | vis | culled | rec (ms) | gpu (ms) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 120 fps | 14848 | 123.73 | 8.08 | 8.95 | 14848 | 1 | 1 | 14848 | 0 | 7.72 | 6.68 |
| 100 fps | 17408 | 100.20 | 9.98 | 10.86 | 17408 | 1 | 1 | 17408 | 0 | 9.50 | 7.93 |
| 75 fps | 22528 | 75.76 | 13.20 | 14.37 | 22528 | 1 | 1 | 22528 | 0 | 12.63 | 10.42 |
| 60 fps | 25600 | 67.68 | 14.78 | 15.75 | 25600 | 1 | 1 | 25600 | 0 | 14.09 | 11.83 |
| 30 fps | 38912 | 37.76 | 26.48 | 36.06 | 38912 | 1 | 1 | 38912 | 0 | 27.44 | 17.49 |
| 24 fps | 47104 | 28.42 | 35.19 | 44.57 | 47104 | 1 | 1 | 47104 | 0 | 35.64 | 21.22 |

Indicators at the pipeline limit: every threshold is solved by the same cached
probe, `1000` meshes with `32` pipelines — 1365.52 fps (p50), p50 0.73 ms,
p95 1.07 ms, 1000 draws, 418 pipeline and 418 descriptor binds per frame,
`vis` 1000, `culled` 0, `rec` 0.60 ms, `gpu` 0.44 ms.

Interpretation:

- **Recording is the wall.** At every mesh limit `rec_ms` is within a few
  percent of `p50` (7.72 of 8.08 ms at 120 fps, 35.64 of 35.19 ms at 24 fps),
  at a flat 0.52–0.56 µs per draw up to 25 600 meshes. The boundary is
  CPU-side draw-list and command recording, not presentation.
- **The GPU pass is second, and its margin shrinks.** `gpu_ms` per draw is
  flat (0.45 µs) across the whole sweep, so it grows linearly with the scene:
  it takes 83 % of the frame budget at 120 fps but only 60 % at 24 fps, while
  `rec_ms` grows faster (1.16× the GPU time at 120 fps, 1.68× at 24 fps).
- **Above ~25 600 meshes the per-draw cost rises** (0.55 µs at 25 600, 0.70 µs
  at 38 912, 0.76 µs at 47 104) and the tail with it: between the 60 fps and
  the 24 fps limit `p50` doubles (14.78 → 35.19 ms) but `p95` nearly triples
  (15.75 → 44.57 ms) and `fps_1pct_low` falls from 35.6 to 10.5 fps.
- **The pipeline axis hits the search bound, not the engine.** 32 pipelines at
  1000 meshes still holds 1365 fps, so the limit is *at least* 32 and the axis
  is unsolved: raise `--pipelines-max` before reading anything into it. The
  bind cost is measurable even at that size — 418 pipeline and 418 descriptor
  binds per frame instead of 1, with `rec_ms` 0.31 → 0.60 ms and `gpu_ms`
  0.27 → 0.44 ms at the same 1000 meshes.
- **Scene upload scales worse than the frame.** `init_ms` stays at 120–136 ms
  at every limit (device and pipeline setup do not depend on the scene), but
  `scene_ms` grows from 6.2 s at 14 848 meshes (0.42 ms/mesh) to 64.2 s at
  47 104 (1.36 ms/mesh).

## 6. Variations worth running

- `--instancing on` / `--indirect on` — batching overhead on the same scene
  (see the note on `instanced` above: this workload does not share geometry).
- `--culling off` — culling gain at the limit, with the grid still fitted.
- `EVAN_MSAA=4` exported — bandwidth cost of multisampling at the limit.
- `EVAN_MULTIVIEW=on` on OpenXR hardware — per-eye cost (the desktop harness
  uses one view).
- Different `--base-meshes` / `--base-pipelines` — the other dimension is held
  fixed at that value, so a limit is only comparable at the same base.
- `--window` — pixel cost. The frame rate of a mesh-limited run should be
  nearly resolution-independent; if it is not, the run is GPU-bound.

## 7. Limitations

- A desktop session is required; the harness is not meant for CI.
- The pipeline axis reuses one SPIR-V module under several names: it measures
  pipeline/state cost, not per-shader GPU cost. Extend the example with real
  shader variants if shader complexity itself is under test.
- The mesh axis uses one shared cube geometry; vertex/index bandwidth per mesh
  is constant. Use larger meshes to shift the balance towards the GPU.
- One process per configuration means driver caches, and therefore `init_ms`,
  improve after the first probe of a session. Compare probes from the same
  session, on an idle machine.
- Debug builds currently emit pre-existing validation messages unrelated to
  this harness (`VK_DYNAMIC_STATE_CULL_MODE` without
  `VK_EXT_extended_dynamic_state`, and `vkFreeDescriptorSets` on a pool created
  without `VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT`). The `-macos`/
  `-linux` base and PoC examples emit the same ones; they do not affect the
  measured record, which the harness parses from its own line.
