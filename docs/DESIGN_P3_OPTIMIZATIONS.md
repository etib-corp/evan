# Design: P3 optimizations — data-oriented storage, frame-build scratch and advanced Vulkan features

Status: partially implemented (WS1–WS3 landed on this branch;
WS4–WS6 gated — see §9)
Scope: evan (`Renderer`, `RenderObject`, `GPUMesh`, `Frame`, `DeviceContext`/
`ADeviceBackend`, `TransferManager`); guillaume/xider only through the
unchanged public `Engine` API (plus opt-in shader artifacts for the bindless
item)
Satisfies: the P3 roadmap of `docs/PERFORMANCE_AUDIT_EVAN.md` (issue #120)
Depends on: v2026.1.4 — the P0/P1/P2 work landed (per-view command buffers,
instance transforms, staging-ring uploads, version-cached resource sync,
pipeline cache, multiview, culling / sorted / instanced / indirect draws) and
the P2-02 instrumentation (PR #157, `docs/DESIGN_INSTRUMENTATION.md`) is
merged. The gated workstreams (§3.4–§3.6) unlock once the audit's N=100/1000
PoC runs on a target workload.

## 1. Goals / non-goals

**Goals**

- Remove the remaining **steady-state heap traffic in the frame-build path**:
  the per-view draw-list and frustum vectors, the `getMeshes()` flatten vector
  (built 2–3 times per frame today), and the per-frame helper vectors in
  `drawFrame`.
- Replace the `std::map<size_t, std::shared_ptr<RenderObject>>` registry with
  a **dense, cache-friendly slot map** (O(1) lookup, linear iteration, stable
  handles) **without changing the public `Engine` API**.
- Fix the remaining **by-value getters**.
- Make the three “advanced Vulkan” items (parallel recording, timeline
  semaphores, bindless) **decision-ready**, each behind an explicit trigger
  from the P2-02 measurements — designed now, implemented when the data says
  so.
- Every workstream lands independently, preserves current behavior, and is
  verifiable with the existing test suites plus the instrumentation.

**Non-goals**

- **No new third-party dependency** (VMA or similar). Any pooling stays
  in-repo and minimal — or is not built at all (see §3.2 notes).
- **No public API/ABI break in this issue.** `RenderObject`/`GPUMesh`
  ownership stays `shared_ptr` for now; migrating to values/`unique_ptr`
  changes public headers and guillaume-facing accessors, so it is staged as a
  follow-up with human approval (AGENTS.md).
- **No mandatory shader-contract change.** Bindless (§3.6) is opt-in; the
  current descriptor layout remains the default.
- **No Vulkan minimum-version bump.** Timeline semaphores and descriptor
  indexing are device-feature-gated with the current paths as fallback.
- No speculative implementation of the gated items before the PoC gate (§4).

## 2. Current state (verified at v2026.1.4)

Two of the audit’s premises are already stale: the `Scene` class no longer
exists (#99) — the object registry lives in `Renderer::_objects` — and the
`materialBound` correctness bug is fixed (the record path tracks the bound
descriptor set per draw range and resolves each item once; see the audit’s
“done” note). What remains of P3:

| # | P3 item | Audit premise | State at v2026.1.4 | Remaining work |
| - | ------- | ------------- | ------------------ | -------------- |
| 1 | Object pools / arenas | staging buffer + device memory alloc/free per add; `vkQueueWaitIdle` per upload; `std::map` node insert | Uploads are batched through the `TransferManager` staging ring (async, one fence per batch, no per-copy queue drain); `sync()` is version-cached; object persistence removed most add/remove churn | One `vkCreateBuffer` + `vkAllocateMemory` pair per vertex and index buffer of every `GPUMesh` (`ADeviceBackend::createBuffer`); heap `RenderObject`/`GPUMesh` + the deep-copied `std::map<uint32_t, Mesh>` argument built per add/update (`Engine::createObject`); per-frame transient vectors (WS1) |
| 2 | SoA / pointer-heavy storage | `shared_ptr<GPUMesh>` maps, AoS | Transforms already reach the GPU through the per-view instance buffer (P2.2) | `std::map<size_t, shared_ptr<RenderObject>>` registry; `vector<shared_ptr<GPUMesh>>` per object; `getMeshes()` flatten-by-value (WS1/WS2) |
| 3 | By-value getters | `getPipelineLayer()`, `getUniformBuffers()` | `getDescriptorSets()` already returns `const&` | `getPipelineLayer()` returns `std::string` by value; `getUniformBuffers()` returns `const std::vector<VkBuffer>` by value and rebuilds it per call (WS3) |
| 4 | Parallel recording | single-threaded, shared command buffer | Per-view primary command buffers can be recorded independently (P2.1/P0.3); the inter-eye serialization is gone | Recording is still serial on the caller thread from one shared `VkCommandPool` (`DeviceContext`); no per-thread pools; no secondary buffers (WS4) |
| 5 | Timeline semaphores | none | Binary `VkFence` per frame slot (`Frame::_inFlight`) + binary semaphores for acquire/present; device feature chains enable only multiview | Entirely open (WS5) |
| 6 | Bindless / descriptor indexing | per-material descriptor sets | One descriptor set per material per frame-in-flight (`GPUMaterial`, pool `materialCount × MAX_FRAMES_IN_FLIGHT`); UBO dynamic + 4 sampler bindings | Entirely open (WS6) |

Also resolved since the audit (no longer in scope): P0.1 logging, P0.2 sleep,
P0.3 fence waits (single end-of-frame wait; desktop pipelined), P0.4 `sync()`
caching, P1.3–P1.7, P2.1–P2.4. Still open elsewhere: P2.5 (`_pipelines`
`std::map` → flat array) and uniform/instance buffer pooling per `GPUMesh`
(§8, open questions).

Measurement prerequisite: the P2-02 instrumentation landed in v2026.1.4
(PR #157): `EVAN_STATS` / `EVAN_STATS_INTERVAL` / `EVAN_STATS_GPU`,
`Renderer::getStats()` and the `docs/PROFILING.md` runbook. What remains is
running the audit’s N=100/1000 PoC on the target workloads; the gates in §4
consume those results.

## 3. Workstreams

Roughly in the issue’s priority order; WS3/WS1/WS2 are ungated (internal, low
risk), WS4–WS6 are trigger-gated.

### 3.1 WS1 — Frame-build scratch reuse (start here)

**Current.** Per frame, steady-state scene: `recordCommandBuffer` allocates a
fresh `std::vector<DrawItem>` (≈200 B/item with the per-item `glm::mat4`) and
a frustum vector per command buffer; `drawFrame` allocates the
`renderableViews`, `acquiredImage` and (multiview) `views` vectors; and
`getMeshes()` builds a by-value `vector<shared_ptr<GPUMesh>>` each of its 2–3
calls per frame.

**Proposal.**

1. A reusable draw scratch owned by the `Renderer`: a small implementation
   struct, defined in `Renderer.cpp` and held in `Renderer.hpp` behind a
   forward declaration + `std::unique_ptr` (so `DrawItem` stays out of the
   public headers). It contains one `std::vector<DrawItem>` and one
   `std::vector<Frustum>` **per view slot** (`MAX_SWAPCHAINS`), plus one
   mesh-pointer list. `clear()` keeps capacity: zero allocations once warmed
   up.
2. Flatten once per frame: `drawFrame` fills the scratch mesh list
   (`std::vector<GPUMesh *>` — no `shared_ptr` copies) and
   `recordCommandBuffer` receives it as a parameter (a `std::span`, like the
   existing `views` parameter). The registry keeps the objects alive for the
   frame’s duration.
3. Replace the `drawFrame` helper vectors with fixed `std::array`s sized
   `MAX_SWAPCHAINS` / `MAX_MULTIVIEW_VIEWS`.

**Why per view slot:** WS4 then has one scratch per recorder with no
cross-thread sharing, and the reuse pattern is visible and testable.

**Acceptance.** `EVAN_STATS` record phase not worse; draw stats identical; no
behavior change; validation clean. Renderer is never copied (verify at
implementation — adding the `unique_ptr` member deletes the implicit copy).

Estimate 1–2 days. Risk low. No approval needed.

### 3.2 WS2 — Dense object registry

**Current.** `Renderer::_objects` is a `std::map<size_t, shared_ptr<RenderObject>>`
(`Renderer.hpp`): O(log N) lookup, one heap node per object, pointer chase
per entry.

**Proposal.** Replace it with an `ObjectRegistry` slot map:

- `std::vector<Slot>`; `Slot { uint32_t generation; std::shared_ptr<RenderObject> object; }`;
  free list of vacant indices.
- The public handle stays an opaque `size_t`: pack `(generation << 32) | index`.
  Removing bumps the generation, so a stale handle fails lookup instead of
  aliasing a newer object (the audit’s “stable indices” requirement).
- `add` amortized O(1); `remove` O(1) + generation bump; `get` O(1);
  iteration is a linear scan over contiguous storage.
- `Renderer::addObject/updateObject/removeObject/getObject` keep their
  signatures; `Engine`’s public verbs are untouched.
- The registry is a standalone header-only template
  (`headers/evan/ObjectRegistry.hpp`) unit-testable without a Vulkan device,
  following the `frameSlotCount` precedent.

**Notes.**

- `RenderObject::_meshes` (`vector<shared_ptr<GPUMesh>>`) stays this cycle:
  WS1 already removes its per-frame cost (no more flatten copies). Migrating
  it to values/`unique_ptr` changes a public header plus the accessor shape
  consumed by guillaume — follow-up issue, human approval.
- Iteration order changes only where slots are reused; the draw sort is a
  total order (`DrawSortKey::sequence`), so visuals stay deterministic.
- The `std::map<uint32_t, Mesh>` deep copy per add/update in
  `Engine::createObject` is adjacent churn; a `std::vector`/span parameter is
  a small optional extra in this PR.

**Acceptance.** New registry unit tests (add/remove/reuse, stale handle,
iteration order, stress); existing `test_render_object`/`test_engine` green.

Estimate 2–3 days. Risk low. No API change. No approval needed.

### 3.3 WS3 — Getter cleanup

- `RenderObject::getPipelineLayer()` → return `const std::string &`
  (no in-repo callers; source-compatible for normal call sites).
- `Renderer::getUniformBuffers()` → cache the handle vector (frames are
  fixed after construction) and return `const std::vector<VkBuffer> &`,
  removing the rebuilt-on-call copy. Call sites (`GPUMaterial` descriptor
  writes) use the result immediately; no lifetime concern.
- Sweep the headers for other `const T getX() const` by-value returns (the
  top-level `const` blocks moves for nothing) and fix them in the same PR;
  list them in the PR description.

**Acceptance.** Builds and existing tests; no consumer changes.

Estimate ≤1 day. Risk low. Can ride along with WS1.

### 3.4 WS4 — Parallel command recording (trigger-gated)

**Trigger.** The P2-02 `recordView` phase exceeds ≈15% of the frame CPU
budget on the target XR workload, or a profile shows recording blocking frame
pacing. Desktop is single-view (no benefit); stereo is two views, so the
ceiling is one view’s record time absent a second recorder.

**Design (when triggered).**

1. Prerequisite: WS1 per-view scratch; per-view instance/indirect slots
   already exist.
2. **Per-thread command pools:** `DeviceContext` grows a small pool set (one
   `VkCommandPool` per recording thread — Vk pools are externally
   synchronized). Each view’s primary buffer is allocated from the pool of
   the thread that records it.
3. `drawFrame` records views on worker threads when
   `renderableViews.size() > 1`, behind an opt-in (`RenderSettings` setter
   following the `EVAN_MULTIVIEW=auto|on|off` pattern). Submissions stay on
   the calling thread after a join (queue is externally synchronized).
4. **Thread-safety contract** for `recordCommandBuffer`: reads only (no
   Renderer state writes), scratch indexed by view slot,
   `_ressourceManager->sync()` hoisted before the parallel section, per-draw
   stats accumulated locally and merged after the join. `EVAN_DEBUG_DRAW_LOG`
   stays serial-gated (the `utility` logger’s thread safety is unproven).

**Secondary command buffers are not required** to record views on separate
threads (concurrent primary recording is legal). They only become interesting
for pre-recorded static subpasses later — explicitly out of scope here.

**Acceptance.** Draw stats identical serial vs parallel; validation clean;
`record` phase drops as expected.

Estimate 3–5 days. Risk medium (shared-state audit). Not this cycle unless
the trigger fires.

### 3.5 WS5 — Timeline semaphores (trigger-gated enabler)

**Trigger.** A concrete ordering need: moving uploads to a transfer queue,
async compute, or measured `inFlightWait`/`renderWait` stalls that a finer
wait would remove. Swapping the fence for its own sake buys nothing today —
P0.3 already collapsed the per-eye waits into one end-of-frame wait.

**Design (when triggered).**

1. **Capability:** `ADeviceBackend::supportsTimelineSemaphore()` via a
   `VkPhysicalDeviceTimelineSemaphoreFeatures` query (core in 1.2,
   `VK_KHR_timeline_semaphore` on 1.1). Both backends already thread a pNext
   feature chain for multiview; timeline joins it. The current fence path
   stays as the fallback — the feature is never assumed.
2. Replace `Frame::_inFlight` with a timeline semaphore + a monotonic submit
   value; slot reuse waits `vkWaitSemaphores(value)`; the OpenXR end-of-frame
   wait likewise. Implementation uses `VkTimelineSemaphoreSubmitInfo` with
   `vkQueueSubmit` to keep the Vulkan 1.1 floor (`vkQueueSubmit2` not
   required).
3. Binary semaphores stay for acquire/present (WSI/runtime interop — not
   replaceable).
4. **Follow-on patch:** `TransferManager` batches signal/await timeline
   values; uploads move to a dedicated transfer queue, removing the
   `flush()`-before-record coupling in `drawFrame`.

**Acceptance.** Fallback path covered by unit tests (feature absent → fence
path); runtime smoke on macOS/MoltenVK and Linux (verify timeline support on
the macOS path at implementation time); the transfer-queue follow-on is
measured separately.

Estimate 2–4 days for the base swap, plus the follow-on. Risk medium.

### 3.6 WS6 — Bindless / descriptor indexing (trigger-gated, last)

**Trigger.** Descriptor binds measurably matter (materials ≫ pipelines on
real content, or hundreds of materials making pool sizing/creation a visible
cost). The draw sort keeps the cost at one bind per material run per view
today, so this is expected to stay the last item.

**Design (when triggered).**

1. **Capability:** `VkPhysicalDeviceDescriptorIndexingFeatures` (core 1.2,
   `VK_EXT_descriptor_indexing` on 1.1); query and enable only when opted in.
   The current descriptor model stays the default.
2. **Engine scaffolding:** a bindless heap (large pool,
   `UPDATE_AFTER_BIND`), a sampled-image array for albedo/normal/metal-rough/
   font atlas, and per-draw material indices carried in the per-frame SSBO
   (or extended push constants). The draw-list builder resolves material →
   slot once per item.
3. **Shader contract:** an opt-in bindless variant of the default/mesh/text
   shaders (sources live in `examples/*/shaders/src/`); external apps keep
   the current binding model.
4. **Lifetime rules:** descriptors are update-after-bind; removal uses
   partially-bound semantics.

**Acceptance.** Bindless and default pipelines render identically on the
sample scenes; the `descriptorBinds` counter drops to ~0 after the first
bind; the supported-device matrix is documented.

Estimate 1–2 weeks. Risk high (shader ecosystem, vendor divergence). Needs
approval (shader contract + descriptor layout).

## 4. Sequencing, triggers and PoC gate

Order: **WS3 → WS1 → WS2** (ungated, internal) → **WS4 / WS5 / WS6** on their
triggers.

Pre-requisite for any gate decision: run the audit’s N=100/1000 PoC on the
now-merged instrumentation (v2026.1.4) with `EVAN_STATS` (and
`EVAN_STATS_GPU` where supported), plus a RenderDoc capture for the GPU side —
the `docs/PROFILING.md` runbook describes the procedure. The trigger table:

| Workstream | Trigger metric (P2-02) | Proposed threshold | If not met |
| ---------- | ---------------------- | ------------------ | ---------- |
| WS4 parallel recording | `recordView` per-view CPU time on XR stereo | > ≈15% of frame CPU budget | keep serial; revisit with more views |
| WS5 timeline semaphores | `inFlightWait` + `renderWait`, or a planned transfer-queue/async-compute feature | any concrete ordering need | keep binary fence; it is sufficient |
| WS6 bindless | `descriptorBinds` per frame vs. material count; pool sizing pressure | binds scale with materials on real content | keep per-material sets; revisit on material growth |

Optional supporting evidence: a `benchmarks/` micro-benchmark for the
registry (WS2) comparing `std::map` vs slot-map iteration at N=1k/10k — cheap
to add with the existing Google Benchmark setup.

## 5. File-by-file change map

Ungated (this cycle, when approved):

| File | WS | Change |
| ---- | -- | ------ |
| `headers/evan/Renderer.hpp` | WS1–WS3 | scratch member (forward-declared struct + `unique_ptr`); cached uniform-buffer vector; `getUniformBuffers()` const&; registry member |
| `sources/Renderer.cpp` | WS1, WS2 | scratch struct + reuse; flatten once per frame; `recordCommandBuffer` mesh-list parameter; `std::array` helpers; removed `getMeshes()` by-value; registry call sites and flatten via `forEach()` |
| `headers/evan/RenderObject.hpp`, `sources/RenderObject.cpp` | WS3 | `getPipelineLayer()` const& |
| `headers/evan/ObjectRegistry.hpp` (new) | WS2 | slot map, header-only template, pure logic |
| `tests/CMakeLists.txt` | WS2 | new test registration |
| `tests/sources/test_object_registry.cpp` (new) | WS2 | registry unit tests |

Gated (own PRs when triggered): `DeviceContext` (per-thread pools, WS4),
`ADeviceBackend` + `glfw/DesktopBackend` + `openxr/XrDeviceBackend`
(feature queries, WS5/WS6), `Frame` (timeline values, WS5),
`TransferManager` (timeline batches, WS5 follow-on), `examples/*/shaders`
(bindless variant, WS6).

AGENTS.md checklist applies per PR: build one platform/backend combo, `ctest`,
`clang-format`, docs/tests in sync, CHANGELOG for visible behavior.

## 6. Test plan

- **WS1:** no behavior change — guarded by the existing suites plus identical
  draw stats (`EVAN_DEBUG_DRAW_LOG` aggregate) before/after; a unit check
  that the scratch keeps capacity across two record calls (pure CPU, no
  device needed if the scratch type is exposed to tests).
- **WS2:** dedicated unit tests (following `test_frame_slots.cpp`
  conventions — pure logic, no device): add/remove/reuse, stale-handle
  rejection, generation bump, dense iteration order, stress loop; existing
  `test_engine`/`test_render_object` unchanged.
- **WS3:** compile + existing tests; no new tests required.
- **WS4:** draw stats identical serial vs parallel; validation layer run;
  race check with a repeated-run harness; only after the trigger.
- **WS5:** fallback-path tests for the feature gate; runtime smoke; only
  after the trigger.
- **WS6:** render-compare bindless vs default on sample scenes; counter
  assertions; only after the trigger.

## 7. Risks and mitigations

- **Cross-cutting reach.** The issue anticipated kernel/ECS/guillaume
  involvement; at v2026.1.3 the scope is much smaller: the registry and the
  frame-build path are `Renderer`-internal, and guillaume/xider only see the
  unchanged `Engine` API. The one consumer-visible change (WS6 shaders) is
  opt-in.
- **Ownership/ABI changes.** Removing `shared_ptr` from `RenderObject`/
  `GPUMesh` storage is deliberately deferred (§3.2); it needs human approval
  and a guillaume/xider migration plan before it can be scheduled.
- **Overlap with prior refactors.** The issue warned about conflicts with
  P1-02/P2-01; both have landed, and WS1/WS2 are designed on top of their
  results (per-view buffers, instance transforms), not beside them.
- **Thread-safety (WS4).** Mitigated by the per-view-slot scratch design, the
  explicit read-only recording contract, and keeping the feature opt-in until
  proven.
- **Feature-gate divergence (WS5/WS6).** Mitigated by device queries with
  fallbacks, never assuming support; macOS/MoltenVK behavior is verified
  before enabling anything by default.
- **Measurement dependency.** The instrumentation is merged (v2026.1.4); the
  gates now wait only for the PoC runs. Until then the gated items stay
  unimplemented — which is the intended default.

## 8. Open questions (maintainer decisions)

1. Is the `shared_ptr` ownership migration (values/`unique_ptr`, true SoA
   `RenderObject[]`) explicitly out of scope for #120? (This proposal says
   yes: stage it as its own issue with approval.)
2. WS4 opt-in surface: environment variable only, `RenderSettings` setter, or
   both? (Proposal: both, following the `EVAN_MULTIVIEW` precedent.)
3. WS6 long-term direction: keep the classic descriptor model as the
   permanent default (bindless as opt-in), or plan a migration once the
   sample shaders are ported? (Proposal: opt-in permanently for now.)
4. Should the per-`GPUMesh` `vkAllocateMemory` loop (§2 row 1) be pooled via
   a minimal in-repo block allocator, or is that too invasive for the actual
   add/update rates? (Proposal: measure first — `meshCreations`/
   `meshDestructions` counters — then decide; not part of WS1–WS3.)
5. P2.5 (`_pipelines` map → flat array) is adjacent to WS2 work; include it
   in the same cycle or keep it separate as planned?

## 9. Implementation status

- **WS3 — getters: implemented (2026-10-16).**
  `RenderObject::getPipelineLayer()` returns `const std::string &`;
  `Renderer::getUniformBuffers()` returns a reference to a handle list cached
  at construction. The sweep found no other by-value getters.
- **WS1 — frame scratch: implemented (2026-10-16).** `DrawListScratch`
  (defined in `Renderer.cpp`, forward-declared in `Renderer.hpp`) owns one
  draw list and one frustum list per view slot, the flattened mesh list (raw
  `GPUMesh *`, no `shared_ptr` copies) and the multiview view list;
  `recordCommandBuffer` takes the mesh list as a `std::span` parameter and
  `drawFrame` uses fixed arrays for its helper buffers instead of vectors.
  The by-value `Renderer::getMeshes()` helper was removed. Validation:
  355/355 unit tests; macOS GLFW smoke run with `EVAN_STATS` (the macOS
  example registers no meshes — the mesh-populated example is Linux-only —
  so only the empty-draw-list path was exercised at runtime here).
- **WS2 — dense registry: implemented (2026-10-16).** `evan::ObjectRegistry<T>`
  (`headers/evan/ObjectRegistry.hpp`, header-only) replaces the
  `std::map<size_t, std::shared_ptr<RenderObject>>` registry: packed
  `(generation << 32) | index` handles, O(1) resolution, amortized O(1)
  add/remove with slot recycling, linear `forEach()` iteration for the
  per-frame flatten, and stale-handle rejection. The renderer's five
  registry call sites and the `drawFrame` flatten use it; `_nextObjectID`
  is gone. Validation: 9 new `ObjectRegistryTest` unit tests (reuse,
  stale-handle rejection, iteration order, 10k-op stress) plus the full
  suite (364/364).
- **WS4–WS6: not started**, gated on the PoC measurements (§4).
