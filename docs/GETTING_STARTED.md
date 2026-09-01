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

The `evan::Engine` receives the platform and prepares Vulkan resources:

```cpp
#include <evan/Engine.hpp>

// Construct the engine with a platform implementation.
```

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
