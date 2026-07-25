# Evan Technical Choices

## Language

Evan is written in C++20 to keep the engine portable while still giving direct
control over memory, performance, and platform integration.

## Graphics Stack

- Vulkan is the primary rendering API.
- GLSL is used for shader sources.
- GLM provides math helpers for transforms and matrices.
- stb is used for lightweight asset and image support.

## Platform Strategy

- OpenXR is used for XR targets.
- GLFW is used for desktop windowing and input.
- Android-specific Vulkan and OpenXR paths are kept separate from desktop
  builds.

## Build Model

Evan is configured by the workspace root CMake project. The engine is compiled
with one backend and one platform selected so the resulting binary stays
explicit and predictable.
