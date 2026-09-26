# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

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
  coverage reporting.
- Documentation: full README, Getting Started tutorial, architecture diagrams,
  and versioning & support policy.
- A runnable `examples/scene_objects` sample.
- `evan::RenderSettings`, passed to the `Engine` constructor, exposes the
  construction-time render configuration: the MSAA sample count and the opaque
  draw ordering.

### Changed

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
