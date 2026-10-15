# Design: frame instrumentation — CPU phases, counters and GPU timestamps

Status: implemented (see §11; N=100/1000 PoC runs pending)
Scope: evan (`Renderer`, `Frame`, `DeviceContext`/`ADeviceBackend`,
`TransferManager`, `RessourceManager`, `GPUMesh`, `EvanPlatform`) plus a new
`docs/PROFILING.md` runbook
Satisfies: the instrumentation issue (P2 — prerequisite for validating every
P0/P1 finding in `docs/PERFORMANCE_AUDIT_EVAN.md`)
Depends on: the current `Renderer::drawFrame`/`Frame` layout (multiview,
instance transforms, per-view command buffers and the single end-of-frame
fence are already landed)

## 1. Goals / non-goals

**Goals**

- Instrumentation **off by default**: the disabled path is a predictable
  branch per hook, no clock reads, no formatting, no allocation. Enabled cost
  in the low microseconds per frame (measured in the acceptance of Phase 1).
- A CPU breakdown of `Renderer::drawFrame` that separates CPU record time from
  GPU wait time: transfer flush, preprocess, resource acquire, in-flight fence
  wait, per-view uniform update / command recording / queue submit, OpenXR
  end-of-frame wait, present, postprocess.
- Per-frame counters: total/visible/culled meshes, draw calls, pipeline binds,
  descriptor binds, `sync()` calls and skips, `GPUMesh` creations/destructions,
  blocking upload waits (the current-code equivalent of the audit's
  `vkQueueWaitIdle` count, see §2).
- GPU render-pass time per recorded pass through a `VkQueryPool` +
  `vkCmdWriteTimestamp`, read back with
  `VK_QUERY_RESULT_WITH_AVAILABILITY_BIT` — never blocking.
- Exposure as **one aggregated log record every N frames**, plus a structured
  accessor (`Renderer::getStats()`) so an application can draw its own overlay
  without evan depending on any UI.
- Documentation: a `docs/PROFILING.md` runbook for RenderDoc, Nsight Graphics /
  Radeon GPU Profiler and `VK_LAYER_KHRONOS_validation` + profiles/performance
  layers, including the N=100 / N=1000 PoC procedure.

**Non-goals**

- An in-engine overlay widget. evan has no UI and should not grow one; the
  accessor is the boundary, the overlay is host-application work
  (guillaume/xider).
- Per-mesh or per-draw timers/logging — that would re-create the P0-01
  hot-loop cost this instrument exists to measure. Fine-grained per-draw GPU
  attribution is RenderDoc/RGP territory.
- GPU-side counters beyond per-pass timestamps, memory profiling, telemetry
  upload, multithreaded recording instrumentation.

## 2. Current state (verified)

`Renderer::drawFrame` (`sources/Renderer.cpp`) today, in order:

1. `_deviceContext->getTransferManager().flush()` — submits pending upload
   batches (non-blocking);
2. `getDeviceBackend()->preprocessFrame(swapchainContext)` — OpenXR:
   `xrWaitFrame` / `xrBeginFrame` / `xrLocateViews`;
3. `vkWaitForFences(frame._inFlight)` — slot-reuse wait;
4. `frame.ensureInstanceCapacity(...)`;
5. acquire loop: `swapchainContext.aquireImage()` + `waitForImage()` —
   `xrAcquireSwapchainImage` / `xrWaitSwapchainImage` (XR,
   `XR_INFINITE_DURATION`) or `vkAcquireNextImageKHR` (desktop);
6. per view: `updateUniformBuffer` / `updateMultiviewUniformBuffer`,
   `frame.resetCommandBuffer`, `recordCommandBuffer` (which calls
   `_ressourceManager->sync()` at entry), `vkQueueSubmit` (the in-flight fence
   rides only the last view);
7. OpenXR only: `vkWaitForFences(frame._inFlight)` before releasing the
   swapchain images / `xrEndFrame`;
8. present loop: `processFrame` per swapchain (`xrEndFrame` /
   `vkQueuePresentKHR`);
9. `postprocessFrame`, then the `_currentFrameIndex` rotation.

Existing partial instrumentation:

- `sources/Renderer.cpp` anonymous namespace: a `DrawStats` struct
  (`totalMeshes`, `visibleMeshes`, `culledMeshes`, `drawCalls`,
  `instancedDraws`, `pipelineBinds`, `descriptorBinds`, `skippedMeshes`)
  computed every frame in `recordCommandBuffer`, emitted only under
  `EVAN_DEBUG_DRAW_LOG` (`isDrawLogEnabled()`). This design makes it permanent.
- Environment-variable convention: `EVAN_MULTIVIEW` (`EvanPlatform.hpp`,
  parsed once through a function-static lambda), `EVAN_DEBUG_DRAW_LOG` /
  `EVAN_DEBUG_DISABLE_CULLING` (non-empty and not `"0"` = on), `EVAN_MSAA`
  (`DeviceContext.cpp`).
- `Frame` owns the per-frame-in-flight resources (command buffers, semaphores,
  fence, uniform/instance/indirect buffers). `MAX_FRAMES_IN_FLIGHT = 2`, so
  there are exactly two slot sets to size a query pool for.
- Blocking device waits in the current code: `TransferManager::waitIdle()`
  (upload staging drain, also called from `GPUMesh::updateVertices` and
  `GPUTexture`), `ADeviceBackend::endSingleTimeCommands()`
  (`vkWaitForFences` per one-time submission), and the two `drawFrame` fence
  waits. The audit's "`vkQueueWaitIdle` on every upload" no longer exists
  verbatim — the counter maps to the first two sites.
- Logger: `utility::logging::Logger` defaults to `FlushPolicy::BUFFERED`
  (debug/info buffered, warnings/errors flushed). One aggregate `info` record
  per interval therefore does not flush per sample.

## 3. Configuration and gates

| Variable | Default | Meaning |
| -------- | ------- | ------- |
| `EVAN_STATS` | off | Master switch. Any non-empty, non-`0` value enables CPU timers, counters and the interval log. |
| `EVAN_STATS_INTERVAL` | `120` | Frames per report window, clamped to ≥ 1; invalid values fall back to the default (120 ≈ 2 s at 60 Hz). |
| `EVAN_STATS_GPU` | on with stats | `0` disables timestamp queries even when supported, so the enabled cost of the GPU path can be attributed in isolation. |

Readers live in `EvanPlatform.hpp` next to `multiviewMode()`, same
static-lambda-once pattern.

Device gate: timestamps are used only when the physical device reports
`VkPhysicalDeviceLimits::timestampComputeAndGraphics == VK_TRUE` and
`timestampValidBits > 0`; otherwise `FrameStats::gpuAvailable = false` and the
report prints `gpu=--`. `timestampPeriod` (ns per tick, may be fractional) is
read from the `VkPhysicalDeviceProperties` already fetched in the backends.

## 4. Design

### 4.1 Data model

`headers/evan/FrameStats.hpp` (new) — fixed size, no heap traffic on the hot
path:

```cpp
namespace evan
{
	/** One accumulated CPU phase (nanoseconds). */
	struct PhaseStats {
		uint64_t lastNs = 0;    ///< most recent sample
		uint64_t totalNs = 0;   ///< window sum
		uint32_t samples = 0;   ///< window sample count
		// add(ns), resetWindow(), averageNs()
	};

	struct FrameStats {
		bool enabled = false;      ///< EVAN_STATS
		bool gpuAvailable = false; ///< device gate + timestamp pool created

		// Frame-level CPU phases, one sample per frame (zero samples allowed)
		PhaseStats transferFlush, preprocess, inFlightWait, acquire,
			updateUniform, record, submit, renderWait, present, postprocess;

		// Per view-slot detail (eye0 / eye1), MAX_SWAPCHAINS entries
		std::array<PhaseStats, MAX_SWAPCHAINS> updateUniformView, recordView,
			submitView, gpuPass;

		// Counters, accumulated over the window
		uint64_t frames = 0;
		uint64_t totalMeshes = 0, visibleMeshes = 0, culledMeshes = 0,
			skippedMeshes = 0;
		uint64_t drawCalls = 0, instancedDraws = 0;
		uint64_t pipelineBinds = 0, descriptorBinds = 0;
		uint64_t syncCalls = 0, syncSkips = 0;
		uint64_t meshCreations = 0, meshDestructions = 0;
		uint64_t uploadWaits = 0;
	};
}
```

`resetWindow()` zeroes the samples and counters but keeps `enabled` /
`gpuAvailable`. `averageNs()` returns 0 for a phase with no sample, which the
formatter renders as `--`.

### 4.2 CPU phase timers

`ScopedTimer` (declared in `FrameStats.hpp`, defined in `FrameStats.cpp`)
holds a `PhaseStats *` that is null when stats are disabled: constructor and
destructor perform one predictable branch and only call
`std::chrono::steady_clock::now()` when active.

| Phase | Measured window (call site in `drawFrame`) |
| ----- | ------------------------------------------ |
| `transferFlush` | around `getTransferManager().flush()` |
| `preprocess` | around `deviceBackend->preprocessFrame(...)` (includes `xrWaitFrame` pacing) |
| `inFlightWait` | around `vkWaitForFences(frame._inFlight)` at slot reuse |
| `acquire` | around the whole acquire loop (`aquireImage` + `waitForImage`, one sample per frame) |
| `updateUniform` | around `updateUniformBuffer`/`updateMultiviewUniformBuffer`, per view, also added to `updateUniformView[slot]` |
| `record` | around `resetCommandBuffer` + `recordCommandBuffer` (therefore includes `RessourceManager::sync()`), per view, also `recordView[slot]` |
| `submit` | around `vkQueueSubmit`, per view, also `submitView[slot]` |
| `renderWait` | around the OpenXR end-of-frame `vkWaitForFences` (no samples on GLFW) |
| `present` | around the whole `processFrame` loop (`xrEndFrame` / `vkQueuePresentKHR`) |
| `postprocess` | around `deviceBackend->postprocessFrame(...)` |

Cost: ~10 `steady_clock::now()` pairs per frame (≈ 20–50 ns each on the
target platforms) plus a handful of integer adds — well under 1 µs/frame
against an 11–16 ms budget. Disabled: branch only, no clock reads.

### 4.3 Counters

| Counter | Increment site | Notes |
| ------- | -------------- | ----- |
| mesh / draw / bind counters | the existing `DrawStats` computation in `recordCommandBuffer`, folded into `_stats` | zero new work; the `EVAN_DEBUG_DRAW_LOG` line keeps reading the same values |
| `syncCalls` / `syncSkips` | `RessourceManager::sync()` — one increment at entry, one on the version short-circuit early-out | distinguishes "called but skipped" from "did work" for P0.4 |
| `meshCreations` / `meshDestructions` | `GPUMesh` constructor/destructor | measures the per-object-add churn driven by guillaume |
| `uploadWaits` | `TransferManager::waitIdle()` when it actually blocks on `_pending`, and `ADeviceBackend::endSingleTimeCommands()` | the current-code replacement for the issue's `vkQueueWaitIdle` counter (see §2) |
| frame fence waits | not counted — timed as `inFlightWait` / `renderWait` phases | |

Implementation: process-wide relaxed `std::atomic<uint64_t>` counters defined
in `sources/FrameStats.cpp`, exposed as inline functions
(`evan::stats::countSync()`, `countMeshCreation()`, `countUploadWait()`, …)
and read once per frame as deltas (`takeGlobalDeltas(FrameStats&)`). This
avoids threading a `FrameStats` pointer through `RessourceManager`, `GPUMesh`,
`TransferManager` and the backends. Every increment is additionally gated
behind the cached `statsEnabled()` bool, so with stats off each site is one
predictable branch; with stats on it is `fetch_add(relaxed)` (single-digit ns).

### 4.4 GPU timestamps

- **Pool**: one `VkQueryPool` per `Frame` (therefore per frame-in-flight),
  `2 × _perFrameSlotCount` queries, `VK_QUERY_TYPE_TIMESTAMP`, created only
  when stats and the device gate are on; destroyed in `Frame::cleanup()`,
  which is already idempotent.
- **Recording**, in `recordCommandBuffer` for view slot `v`:
  - `vkCmdResetQueryPool(cb, pool, 2v, 2)` right after `vkBeginCommandBuffer`;
  - `vkCmdWriteTimestamp(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 2v)`
    immediately before `vkCmdBeginRenderPass`;
  - `vkCmdWriteTimestamp(cb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 2v+1)`
    immediately after `vkCmdEndRenderPass`.
- **Readback**, in `drawFrame`, immediately after the slot's
  `vkWaitForFences(...frame._inFlight...)` succeeded — the submissions that
  wrote these queries have completed at that point.
  `vkGetQueryPoolResults(..., VK_QUERY_RESULT_64_BIT |
  VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)`; `VK_NOT_READY` or availability 0
  means no sample this frame. `VK_QUERY_RESULT_WAIT_BIT` is never used.
  Samples are stored in `gpuPass[slot]` after conversion
  `ns = ticks × timestampPeriod`. On devices with `timestampValidBits < 64`,
  read 32-bit results and mask with `(1u << validBits) - 1`.
- **Safety**: the reset is *recorded*, so it executes in queue order and only
  after the previous use of that frame slot has completed (the existing fence
  guarantees this); the readback happens only after the same fence. No host
  reset, no new synchronization, no behavior change on the render path.
- **Multiview**: one recorded pass covers both eyes, so the pair gets one
  sample (reported as `pass0`). Per-eye GPU numbers are taken with
  `EVAN_MULTIVIEW=off` — the audit's existing A/B path; documented in
  `PROFILING.md`.
- **Cost**: one pool per frame-in-flight, one reset + two timestamp writes per
  recorded pass (nanoseconds on the GPU), one small readback (2–8 `u64`) per
  frame.

### 4.5 Reporting and API

- `Renderer::reportStatsIfDue()` is called once at the end of `drawFrame`,
  after the frame-index rotation. When `_stats.frames` has reached
  `statsLogInterval()`, it emits exactly **one** `info()` record (built with a
  single `<<` stream, no `std::endl`, no explicit `flush()` — consistent with
  `FlushPolicy::BUFFERED`), then resets the window. Nothing ever logs inside
  the acquire/record loops.
- Line sketch (single record, average values in ms):

```
evan stats: frames=120 | cpu_ms(avg): flush=0.01 pre=0.02 acq=0.05 wait=1.02 ubo=0.01 rec=0.84 submit=0.03 renderwait=-- pres=0.04 post=0.00 | gpu_ms(avg): pass0=3.21 pass1=3.19 | meshes: total=1000 vis=998 culled=2 skip=0 | draws: calls=998 inst=420 pso=12 ds=12 | sync: calls=240 skip=238 | mesh_churn: +12/-12 | uploadwaits=1
```

  `--` marks phases with no samples on this backend (e.g. `renderwait` on
  GLFW, `gpu` without timestamp support).
- Structured access (the app-facing boundary, usable for a host-side debug
  overlay):
  - `const FrameStats &Renderer::getStats() const noexcept;`
  - `void Renderer::resetStats() noexcept;` (window reset)
  - optional thin passthrough `Engine::getFrameStats()` for hosts that only
    talk to `Engine`.

  The accessor returns the live accumulator; the reporter resets it after
  logging, so the log and an overlay consume the same data without double
  ownership.

## 5. File-by-file change map

| File | Change |
| ---- | ------ |
| `headers/evan/FrameStats.hpp` | **new** — `PhaseStats`, `FrameStats`, `ScopedTimer`, `evan::stats::count*`/`takeGlobalDeltas` declarations. |
| `sources/FrameStats.cpp` | **new** — counter definitions, delta sampling, window reset. Add to `EVAN_SOURCES_COMMON` in `cmake/Sources.cmake`. |
| `headers/evan/EvanPlatform.hpp` | `statsEnabled()`, `statsLogInterval()`, `statsGpuEnabled()` inline readers (same pattern as `multiviewMode()`). |
| `headers/evan/Renderer.hpp` / `sources/Renderer.cpp` | `FrameStats _stats` member; timers in `drawFrame`; `DrawStats` promoted and folded into `_stats` (the `EVAN_DEBUG_DRAW_LOG` line keeps working); `reportStatsIfDue()`; public `getStats()`/`resetStats()`. |
| `headers/evan/Frame.hpp` / `sources/Frame.cpp` | timestamp query pool creation/destruction, `getTimestampQueryPool()`, `hasTimestampQueries()`. |
| `headers/evan/ADeviceBackend.hpp` / `sources/ADeviceBackend.cpp` | `supportsTimestampQueries()` capability query; `uploadWaits` increment in `endSingleTimeCommands()`. |
| `headers/evan/TransferManager.hpp` / `sources/TransferManager.cpp` | `uploadWaits` increment in `waitIdle()` when it blocks. |
| `headers/evan/RessourceManager.hpp` / `sources/RessourceManager.cpp` | `syncCalls`/`syncSkips` increments in `sync()`. |
| `headers/evan/GPUMesh.hpp` / `sources/GPUMesh.cpp` | creation/destruction counters in constructor/destructor. |
| `headers/evan/DeviceContext.hpp` / `sources/DeviceContext.cpp` | `getTimestampPeriod()` passthrough next to the existing MSAA caps. |
| `sources/Engine.cpp` / `headers/evan/Engine.hpp` | optional `getFrameStats()` passthrough only. |
| `cmake/Sources.cmake`, `tests/CMakeLists.txt` | register `sources/FrameStats.cpp` and `tests/sources/test_frame_stats.cpp` (both lists are explicit). |
| `tests/sources/test_frame_stats.cpp` | **new** — see §7. |
| `docs/PROFILING.md` | **new** — external-tools runbook and PoC procedure (issue point 5). |
| `docs/PERFORMANCE_AUDIT_EVAN.md` | cross-link this design under “Instrumentation recommendations”; fill measured numbers under “Expected impact” once the PoC runs. |
| `CHANGELOG.md` | `Unreleased/Added` entry when implemented (not part of this design-only change). |

Modules from the issue that are deliberately untouched: `Platform`, `Event`,
`utility` (Math/Utils/FileSystem), and the swapchain contexts themselves — the
acquire/present timers wrap their existing call sites in `Renderer`.

## 6. Sequencing and acceptance

**Phase 1 — CPU timers + counters (~2–3 days, the bulk of the value).**
`FrameStats` + env readers + `ScopedTimer`; timers in `drawFrame`; counters
wired; interval reporter; `getStats()`; unit tests.
*Acceptance:* GLFW example with `EVAN_STATS=1 EVAN_STATS_INTERVAL=60` emits
exactly one line per 60 rendered frames whose counters match a known scene;
same binary with `EVAN_STATS=0` shows frame time within noise of the
stats-free baseline (≥ 3000 frames); no log records besides the interval line.

**Phase 2 — GPU timestamps (~1–2 days).**
Capability query + `timestampPeriod`; `Frame` query pool; timestamp writes in
`recordCommandBuffer`; availability-based readback in `drawFrame`; unit tests
for conversion/masking/availability.
*Acceptance:* desktop GPU `gpu_ms` consistent with a RenderDoc capture of the
same frame (±10%); XR runtime (when available) shows per-pass values, and
`EVAN_MULTIVIEW=off` separates the eyes; validation layer clean; no frame ever
blocks on query results (a delayed readback prints `--`, never stalls).

**Phase 3 — docs + PoC (~1 day plus runs).**
`docs/PROFILING.md`; audit cross-links; N=100 and N=1000 runs on at least one
desktop and one XR target; CPU-vs-GPU breakdown plus counter dump published in
the issue and folded into the audit's “Expected impact”.

## 7. Test plan

Unit/integration — `tests/sources/test_frame_stats.cpp`, headless, no Vulkan:

- `PhaseStats`: add/average/resetWindow, empty window → 0,
  `lastNs`/`totalNs`/`samples` bookkeeping.
- Env parsing: value tables for `""`, `"0"`, `"1"`, `"120"`, `"abc"`, `"-3"`
  through pure `statsEnabledFrom(const char *)`-style helpers (no `setenv`).
- Global counters: simulated increments are exactly reflected by
  `takeGlobalDeltas`, deltas reset, no double counting across frames.
- Timestamp math: fractional `timestampPeriod`, 32-bit path with
  `timestampValidBits` masking, `VK_NOT_READY` / availability-0 paths.
- Report triggering: with a stubbed clock/counter the report fires at exactly
  N frames, not N±1.

Runtime checks (manual, documented in `PROFILING.md`):

- Stats on/off A/B on the same binary; validation layers clean with stats on.
- RenderDoc GPU cross-check of `gpu_ms`.
- Multiview on/off eye attribution; XR vs GLFW phase sets (`renderwait` only
  on XR).

## 8. Risks and mitigations

| Risk | Mitigation |
| ---- | ---------- |
| Timestamp support varies (MoltenVK, AMD fractional period, `timestampValidBits < 64`) | Device gate + 32-bit masked path + `--` reporting; stats degrade to CPU-only, never fail. |
| Query reset / readback hazard with frames in flight | Reset is recorded (queue-ordered) and read only after the slot fence signalled; availability bit checked; `WAIT_BIT` never used. |
| Multiview collapses the eyes into one GPU sample | Documented; `EVAN_MULTIVIEW=off` gives per-eye numbers (existing A/B path). |
| Enabled cost creeping up | Fixed hook list (§4.2); no formatting or allocation outside the interval branch; enforced by Phase 1 acceptance. |
| Per-sample logging creeping back (P0-01) | Exactly one log call site, once per window; the per-mesh `EVAN_DEBUG_DRAW_LOG` path stays separate and off. |
| Counter semantics drift | `sync` is two counters (calls/skips), validated against `EVAN_DEBUG_DRAW_LOG` output on a known scene. |
| Logger flushing per record | `FlushPolicy::BUFFERED` already buffers info; the reporter never calls `flush()` or `std::endl`. |
| Readback cost on some drivers | One `vkGetQueryPoolResults` per frame (2–8 `u64`); checked in Phase 2 acceptance. |

## 9. Open questions

1. Runtime toggle (`Renderer::setStatsEnabled` / `Engine`) vs. env-only —
   proposal: env-only in Phase 1; add the setter if the host overlay needs it.
2. Default interval 120 frames — proposal: keep, `EVAN_STATS_INTERVAL`
   overrides.
3. `getStats()` returns the live window; the reporter resets after logging.
   If a host overlay also consumes it, windows interleave — acceptable for a
   debug aid, or gate the reporter off when the host opts out.
4. `Engine::getFrameStats()` passthrough vs. hosts reaching `Renderer`
   directly — proposal: thin passthrough, matching the rest of the public API.
5. Surface mesh-churn counters through `utility::` so guillaume can attribute
   them to render systems? Out of scope here; candidate for the guillaume
   audit follow-up.

## 10. External tooling runbook (deliverable of `docs/PROFILING.md`)

Contents the doc will ship (issue point 5):

- **evan stats**: env variables, log format, how to read CPU-bound (`rec`
  dominant, `wait` small) vs. GPU-bound (`gpu_ms` ≈ frame budget, `wait`
  large) frames.
- **RenderDoc**: capture the GLFW build (launch under RenderDoc or
  `ENABLE_VULKAN_RENDERDOC_CAPTURE=1`), cross-check draw-call and
  descriptor-bind counts against the stats counters, use its GPU timings for
  per-draw attribution.
- **Nsight Graphics** (NVIDIA) and **Radeon GPU Profiler** (AMD): frame
  timing, occupancy and bandwidth confirmation of the GPU hypotheses.
- **Layers**: `VK_LAYER_KHRONOS_validation` (plus sync validation and
  GPU-assisted validation), the profiles layer (`VK_LAYER_KHRONOS_profiles`)
  to pin capabilities and compare code paths, and vendor performance layers
  where available.
- **PoC recipe**: N=100 and N=1000 scenes; table to fill (CPU phases, GPU pass
  time, counters); how to toggle `EVAN_MULTIVIEW`, `EVAN_MSAA`, culling and
  stats to isolate each ranked finding from the audit.
## 11. Implementation status (2026-10-15)

Phases 1 and 2 are implemented; Phase 3 (the N=100/1000 GPU PoC runs) is
pending because it needs measurement sessions on real targets.

Landed:

- `headers/evan/FrameStats.hpp` / `sources/FrameStats.cpp`:
  `PhaseStats`, `FrameStats` (phases, per-view-slot arrays, counters,
  `gpuAvailable`), `ScopedTimer` (null sink when disabled), process-wide
  relaxed-atomic counters with delta sampling and a test-only gate override,
  timestamp masking/conversion helpers.
- `EvanPlatform.hpp`: `statsFlagEnabled()`, `statsIntervalFromValue()`,
  `statsEnabled()`, `statsLogInterval()`, `statsGpuEnabled()`.
- `Renderer`: all ten phase timers in `drawFrame` (with per-view-slot
  `updateUniformView`/`recordView`/`submitView`), draw counters folded from
  the existing `DrawStats` computation in `recordCommandBuffer`, one
  aggregated `info()` record per `EVAN_STATS_INTERVAL` frames (no per-sample
  logging or flushing), `getStats()`/`resetStats()`.
- `Frame`: timestamp query pool (two queries per view slot) created only when
  `EVAN_STATS` + `EVAN_STATS_GPU` + device support allow it; recorded
  `vkCmdResetQueryPool` + `vkCmdWriteTimestamp` around each render pass;
  pooled readback in `drawFrame` after the slot fence with
  `VK_QUERY_RESULT_WITH_AVAILABILITY_BIT`, never blocking.
- `ADeviceBackend`: `supportsTimestampQueries()`, `getTimestampPeriod()`,
  `getTimestampValidBits()` (queue-family property), upload-wait counter in
  `endSingleTimeCommands()`; `TransferManager::waitIdle()` upload-wait
  counter; `RessourceManager::sync()` call/skip counters; `GPUMesh`
  creation/destruction counters.
- `Engine::getFrameStats()` passthrough; tests in
  `tests/sources/test_frame_stats.cpp` (8 cases); `docs/PROFILING.md`
  runbook; `CHANGELOG.md` entry.

Deviations from the plan above:

- `getTimestampPeriod()` / `getTimestampValidBits()` live on
  `ADeviceBackend` instead of `DeviceContext`: `timestampValidBits` is a
  queue-family property and the backend already owns `findQueueFamilies()`.
- The timestamp capability gate also requires `timestampPeriod > 0` and a
  nonzero valid-bits count before pools are created.

Remaining: the N=100 / N=1000 PoC table in `docs/PROFILING.md` §6, the
measurements folded into `PERFORMANCE_AUDIT_EVAN.md` “Expected impact”, and
the optional host overlay consumption of `getStats()` (guillaume/xider side).
