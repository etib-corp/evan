# Getting Started

This tutorial walks you from an empty project to a working Evan setup. It
assumes you have already [built the library](../README.md#building).

## 1. Select a backend and platform

Evan requires exactly one backend (`BUILD_FOR_OPENXR` or `BUILD_FOR_GLFW`) and
one platform (`BUILD_FOR_ANDROID`, `BUILD_FOR_LINUX`, `BUILD_FOR_WINDOWS`, or
`BUILD_FOR_MACOS`).

```sh
cmake -S . -B build -DBUILD_FOR_GLFW=ON -DBUILD_FOR_LINUX=ON
cmake --build build
```

## 2. Create a platform implementation

Evan delegates platform-specific behavior to a `Platform` implementation. For a
desktop GLFW build, use the provided desktop platform; for XR, use the OpenXR
platform.

## 3. Create an engine

The `evan::Engine` receives the platform and prepares Vulkan resources. It also
accepts optional `evan::RenderSettings`: multisampling is disabled by default,
and the sample count is baked into the swapchain render pass, so it can only be
chosen at construction time.

```cpp
#include <evan/Engine.hpp>

// Construct the engine with a platform implementation.
evan::RenderSettings settings;

// Opt back into multisampling when the extra edge quality is worth the
// bandwidth; the default renders at a single sample and resolves nothing.
settings.msaaSamples = VK_SAMPLE_COUNT_4_BIT;

evan::Engine engine(ressourceProvider, platform, settings);
```

`settings.opaqueSort` controls how opaque draws are ordered:

- `OpaqueSortMode::FrontToBack` (default) buckets them by depth so nearer
  geometry is shaded first and occluded fragments are rejected early.
- `OpaqueSortMode::StateFirst` groups them by pipeline and material instead,
  which batches more but pays more overdraw.

The ordering can also be changed at runtime with
`Renderer::setOpaqueSortMode()`. `EVAN_MSAA=1|2|4` overrides the sample count
without a rebuild, which is how the cost of multisampling is measured.

## 4. Populate a scene

Add renderable objects, meshes, and materials to a `Scene`:

```cpp
#include <evan/Scene.hpp>

evan::Scene scene;
// scene.addObject(id, renderObject);
```

## 5. Run the frame loop

The engine drives the frame loop: it polls events, updates state, records
commands, and presents the image.

## Next steps

- Read [How Evan Works](HOW_EVAN_WORKS.md) for the frame lifecycle.
- Browse the [API reference](https://etib-corp.github.io/evan).
