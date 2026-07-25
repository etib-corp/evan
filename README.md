# Evan

Evan is the rendering and runtime layer used by XIDER. It provides the Vulkan
graphics pipeline, platform backends for OpenXR and GLFW, and the frame/update
loop that drives the application.

## What It Provides

- Vulkan-based rendering and swapchain management.
- Desktop and XR platform abstractions.
- Scene, mesh, shader, and material helpers.
- Optional backend integration for OpenXR or GLFW.

## Build Notes

Evan is built from the workspace root through CMake. Select exactly one backend
and one platform when configuring the project.

- Backend: `BUILD_FOR_OPENXR` or `BUILD_FOR_GLFW`
- Platform: `BUILD_FOR_ANDROID`, `BUILD_FOR_LINUX`, `BUILD_FOR_WINDOWS`, or
    `BUILD_FOR_MACOS`

## Documentation

- [How Evan Works](docs/HOW_EVAN_WORKS.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Technical Choices](docs/TECHNICAL_CHOICES.md)
