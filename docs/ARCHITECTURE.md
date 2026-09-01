# Evan Architecture

Evan is split into a small set of runtime layers so rendering code, platform
code, and scene logic stay separated.

## Module Layout

```mermaid
graph TD
    Engine[Engine] --> Platform[Platform]
    Engine --> Device[DeviceContext]
    Engine --> Swapchain[SwapchainContext]
    Engine --> Scene[Scene]

    Scene --> RenderObject[RenderObject]
    RenderObject --> GPUMesh[GPUMesh]
    Scene --> GPUMaterial[GPUMaterial]
    Scene --> GPUShader[GPUShader]

    Platform --> Backend[Backend: OpenXR / GLFW]
```

- **Engine** owns the frame loop and coordinates update, draw, and present.
- **Platform** abstracts the target backend (OpenXR or GLFW) and platform.
- **Device & swapchain** manage Vulkan resources and frame ownership.
- **Scene** holds renderable objects, meshes, and materials.

## Frame Lifecycle

```mermaid
sequenceDiagram
    participant Engine
    participant Platform
    participant Device as DeviceContext
    participant Scene

    loop each frame
        Engine->>Platform: poll events
        Engine->>Scene: update state
        Engine->>Device: record command buffers
        Engine->>Platform: present image
    end
```

## Build Variants

- `BUILD_FOR_GLFW` enables desktop platform support.
- `BUILD_FOR_OPENXR` enables XR support.
- Android-specific paths are enabled when the selected platform is Android.

## Design Goal

Keep the engine low-level enough for performance while still exposing a
straightforward application lifecycle to the higher layers.
