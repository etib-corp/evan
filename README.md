# Evan

Evan is the rendering and runtime layer used by XIDER. It provides the Vulkan
graphics pipeline, platform backends for OpenXR and GLFW, and the frame/update
loop that drives the application.

## What It Provides

- Vulkan-based rendering and swapchain management.
- Desktop and XR platform abstractions.
- Scene, mesh, shader, and material helpers.
- Optional backend integration for OpenXR or GLFW.

## Key Concepts

- **Engine** — owns the frame loop and coordinates update, draw, and present.
- **Platform** — abstracts the target backend (OpenXR or GLFW) and platform
  (Android, Linux, Windows, macOS).
- **Device & swapchain** — manage Vulkan resources and frame ownership.
- **Scene** — holds renderable objects, meshes, and materials.
- **Mesh / shader / material** — describe renderable content.

## Quickstart

Evan is a library; it is consumed by an application (such as XIDER) that
provides a platform implementation. A minimal setup selects one backend and one
platform:

```sh
cmake -S . -B build \
  -DBUILD_FOR_GLFW=ON \
  -DBUILD_FOR_LINUX=ON
cmake --build build
```

## Building

### Dependencies

- CMake 3.10+
- A C++20 compiler
- The Vulkan SDK
- Doxygen + Graphviz (only for `-DBUILD_DOCS=ON`)

Evan fetches its dependencies (Utility, GLM, STB) via CMake `FetchContent`.

### Configure and build

Select exactly one backend and one platform:

```sh
cmake -S . -B build \
  -DBUILD_FOR_GLFW=ON \
  -DBUILD_FOR_LINUX=ON \
  -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Optional options:

- `-DBUILD_TESTING=ON` — build and run the test suite.
- `-DBUILD_DOCS=ON` — build Doxygen documentation.
- `-DBUILD_BENCHMARKS=ON` — build the benchmark harness.

## Consuming as a dependency

Once installed, downstream projects can use `find_package(evan)`:

```sh
cmake --install build --prefix /path/to/prefix
```

```cmake
find_package(evan REQUIRED)
target_link_libraries(my_app PRIVATE evan::evan)
```

## Documentation

- [How Evan Works](docs/HOW_EVAN_WORKS.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Getting Started](docs/GETTING_STARTED.md)
- [Versioning & Support](docs/VERSIONING.md)
- [Technical Choices](docs/TECHNICAL_CHOICES.md)
- [Code Conventions](docs/CODE_CONVENTIONS.md)
- [Commit Conventions](docs/COMMIT_CONVENTIONS.md)

## Contributing

We welcome contributions from the community! If you're interested in
contributing to Evan, please check out our
[Contributing Guidelines](CONTRIBUTING.md) for more information on how to get
involved.

## License

Evan is released under the [MIT License](LICENSE). See
[CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) for our community standards and
[SECURITY.md](SECURITY.md) for reporting vulnerabilities.
