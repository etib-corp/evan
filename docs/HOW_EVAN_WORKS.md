# How Evan Works

Evan runs a platform-driven render loop around a shared engine instance.

## Typical Startup

1. Create the target platform implementation.
2. Construct `evan::Engine` with that platform.
3. Initialize the asset manager when the platform requires it.
4. Enter the application loop and call `update()`, `pollEvents()`, and
	`render()` as needed.

## Runtime Responsibilities

- `pollEvents()` gathers input and platform events.
- `update()` advances scene and application state.
- `render()` records Vulkan commands and presents the current frame.

## Backends

- GLFW is used for desktop windowing and input.
- OpenXR is used for XR-specific platform integration.
- Android uses the Android platform path together with Vulkan support.

## Practical Rule

The higher-level application should own the loop, while Evan owns rendering and
platform-specific graphics work.
