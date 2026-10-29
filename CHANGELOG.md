# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2026.1.0] - 2026-09-26

## [2026.1.1] - 2026-10-01

## [2026.1.2] - 2026-10-08

## [2026.1.3] - 2026-10-15

## [2026.1.4] - 2026-10-16

## [2026.2.0] - 2026-10-22

## [2026.2.1] - 2026-10-29

## [Unreleased]

### Added

- Frame instrumentation, off by default: `EVAN_STATS=1` enables CPU phase
  timers in `Renderer::drawFrame` (upload flush, preprocess, acquire,
  in-flight fence wait, per-view uniform/record/submit, OpenXR end-of-frame
  wait, present, postprocess), per-frame counters (meshes, draw calls,
  pipeline/descriptor binds, `sync()` calls vs skips, `GPUMesh`
  creation/destruction churn, blocking upload waits) and non-blocking GPU
  timestamp queries around each render pass. One aggregated log record is
  emitted every `EVAN_STATS_INTERVAL` frames (default 120); `EVAN_STATS_GPU=0`
  keeps the CPU side without query pools. `Renderer::getStats()` and
  `Engine::getFrameStats()` expose the window for a host-side debug overlay.
  See `docs/DESIGN_INSTRUMENTATION.md` and the `docs/PROFILING.md` runbook
  (RenderDoc, Nsight Graphics, Radeon GPU Profiler, validation + performance
  layers, N=100/1000 PoC recipe).
- Engine: implement `utility::Engine::getDeltaTime()` (exposes the existing
  per-frame `_deltaTime`) and the scissor API — `Engine::setScissor()` /
  `clearScissor()` forward to `Renderer`, which applies the clamped rectangle
  through the dynamic Vulkan scissor when recording frames.
- Standard open-source documentation: `CHANGELOG.md`, `CODE_OF_CONDUCT.md`,
  `CONTRIBUTING.md`, `SECURITY.md`, `AUTHORS.md`, and `LICENSE`.
- Packaging & consumability: `install()`/`export()`, a CMake package config
  (`evanConfig.cmake`) for `find_package(evan)`, and CPack rules.
- Benchmark harness (`BUILD_BENCHMARKS`) covering scene object management.
- CI hardening: `ctest` execution, ASan/UBSan and `clang-tidy` jobs, and
  per-platform coverage reporting (GLFW on Linux, macOS and Windows; OpenXR on
  Linux) merged in Codecov.
- Documentation: full README, Getting Started tutorial, architecture diagrams,
  versioning & support policy, and a [Testing & Coverage](docs/TESTING.md)
  guide.
- A runnable `examples/scene_objects` sample.
- `evan::RenderSettings`, passed to the `Engine` constructor, exposes the
  construction-time render configuration: the MSAA sample count and the opaque
  draw ordering.
- Stereo multiview rendering (`VK_KHR_multiview`), enabled automatically when
  the device exposes the feature and the OpenXR runtime accepts a swapchain
  with one array layer per view: both eyes are drawn in one render pass over a
  single layered swapchain, recorded in one command buffer and submitted once.
  `EVAN_MULTIVIEW=auto|on|off` overrides the selection and the existing
  two-pass path stays as the transparent fallback. See
  `docs/DESIGN_MULTIVIEW.md`.

### Changed

- The per-view uniform block is now `model` followed by `views[2]` (one
  view/projection pair per eye), selected in the vertex shader with
  `gl_ViewIndex`; the two-pass fallback writes each eye into its own dynamic
  slot. Shader assets must be rebuilt against this layout, and the example
  GLSL sources now live under `examples/GLFW-linux-base/shaders/src` with the
  compiled SPIR-V next to them.

- Replaced `file(GLOB)` with explicit source lists for reproducible builds.
- Multisampling is now opt-in: `DeviceContext` defaults to
  `VK_SAMPLE_COUNT_1_BIT`, a single-sampled desktop swapchain renders straight
  into the swapchain image with no resolve pass, and `RenderSettings::msaaSamples`
  or `DeviceContext::setMsaaSamples()` re-enable 2× or 4×. `EVAN_MSAA=1|2|4`
  overrides both for measurements. The OpenXR context still honors the
  runtime's recommended sample count when the runtime only offers multisampled
  swapchain images.
- Graphics pipelines are split into an opaque and an alpha-blended variant per
  shader, selected from the material's alpha semantics
  (`utility::graphic::Material::getAlphaMode`, overridable per shader with
  `Renderer::setShaderBlendMode`). Blending is disabled for opaque geometry,
  blended draws no longer write depth, opaque draws are ordered front-to-back
  and blended draws back-to-front. This doubles the number of pipelines, which
  the pending `VkPipelineCache` work should absorb.
- `GPUMaterial`/`GPUTexture` now use the public `Texture::type()`/`pixels()`
  accessors instead of accessing protected members.
- Removed the hardcoded 60 FPS sleep in `Engine::updateDeltaTime()`; the loop
  is now paced by the presentation mechanism (vsync on desktop, `xrWaitFrame`
  on OpenXR), with an opt-in `Engine::setTargetFps()` limiter.
- Desktop swapchains now prefer `VK_PRESENT_MODE_FIFO_KHR` (vsync) over
  `VK_PRESENT_MODE_MAILBOX_KHR` so the loop is paced by default.

## [1.0.0] - 2025-08-21

### Added

- Initial release of the Evan rendering and runtime layer.
- Vulkan-based rendering and swapchain management.
- Desktop and XR platform abstractions (OpenXR and GLFW).
- Scene, mesh, shader, and material helpers.
- Optional backend integration for OpenXR or GLFW.
