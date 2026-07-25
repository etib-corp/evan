# Evan Architecture

Evan is split into a small set of runtime layers so rendering code, platform
code, and scene logic stay separated.

## Main Layers

- Platform interfaces describe how the engine talks to Android, GLFW, or XR.
- Device and swapchain classes manage Vulkan resources and frame ownership.
- Scene, mesh, shader, and material classes describe renderable content.
- The engine owns the frame loop and coordinates update, draw, and present.

## Data Flow

1. A platform implementation is created for the target build.
2. `evan::Engine` receives that platform and prepares Vulkan resources.
3. Scenes populate meshes and materials.
4. Each frame updates state, records commands, and presents the image.

## Build Variants

- `BUILD_FOR_GLFW` enables desktop platform support.
- `BUILD_FOR_OPENXR` enables XR support.
- Android-specific paths are enabled when the selected platform is Android.

## Design Goal

Keep the engine low-level enough for performance while still exposing a
straightforward application lifecycle to the higher layers.
