# AGENTS.md

## Repository purpose

EVAN is a C++ rendering/engine library for building desktop and extended reality (XR) applications. The repository contains a modular C++ codebase with platform abstraction layers (GLFW/OpenXR backends) and a Vulkan-based renderer. Agents should focus on maintaining modularity, following existing patterns, and respecting the platform/backend build matrix.

## Tech stack

- **Main language**: C++20 (`CMAKE_CXX_STANDARD 20`)
- **Build system**: CMake 3.10+ (no presets; uses platform/backend options)
- **Dependency strategy**: FetchContent (glm, stb, GoogleTest, OpenXR-SDK, GLFW, doxygen-awesome-css)
- **Test framework**: GoogleTest via `gtest_discover_tests()`
- **Formatter**: clang-format (LLVM-based, 80-column, tabs, 4-space indent)
- **CI**: GitHub Actions (Linux/macOS/Windows runners, Vulkan SDK setup, Android support)
- **Documentation**: Doxygen with doxygen-awesome-css theme

## Project layout

- `CMakeLists.txt` — root build configuration with platform/backend option validation
- `cmake/` — platform configs (`Config*.cmake`) and `Docs.cmake` module
- `headers/` — EVAN public headers under `evan/`
- `sources/` — EVAN implementation sources
- `tests/` — EVAN unit tests
- `examples/` — example applications
- `docs/` — architecture docs, build guide, code/contribution conventions
- `scripts/` — helper scripts (`run-clang-format.sh`, `install_ubuntu_dependencies.sh`)
- `build/` — CMake build output (auto-generated; do not edit)

## Build and test

### Preferred workflow

```bash
# Configure (Linux + GLFW example)
cmake -S . -B build \
  -DBUILD_FOR_GLFW=ON \
  -DBUILD_FOR_LINUX=ON \
  -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build -j

# Test (if BUILD_TESTING=ON)
ctest --test-dir build --output-on-failure
```

**Build matrix rules** (enforced by root `CMakeLists.txt`):

- Enable **exactly one** platform: `BUILD_FOR_ANDROID`, `BUILD_FOR_LINUX`, `BUILD_FOR_WINDOWS`, or `BUILD_FOR_MACOS`
- Enable **exactly one** backend: `BUILD_FOR_OPENXR` or `BUILD_FOR_GLFW`

### Other useful commands

```bash
# Format all C++ files
./scripts/run-clang-format.sh

# Install Ubuntu dependencies (CI-like setup)
bash scripts/install_ubuntu_dependencies.sh

# Build with tests enabled
cmake -S . -B build -DBUILD_TESTING=ON <platform/backend options>

# Build documentation
cmake -S . -B build -DBUILD_DOCS=ON <platform/backend options>
```

## C++ conventions for agents

- **Header/source placement**:
  - Public headers: `headers/evan/` (e.g., `headers/evan/Engine.hpp`)
  - Implementation: `sources/`
  - Tests: `tests/sources/`
- **File extensions**: Use `.hpp` for headers, `.cpp` for sources
- **Naming**:
  - Classes/structs: `PascalCase`
  - Functions/variables: `camelCase`
  - Member fields: prefix with `_` (e.g., `_value`, `_sharedValue`)
  - Namespaces: nested by module (e.g., `evan`, `evan::glfw`)
- **Class layout** (in headers):
  1. `public` section (types, constructors, methods, fields)
  2. `protected` section (same order)
  3. `private` section (same order)
- **Error handling**: Use exceptions for errors; validate inputs explicitly; avoid raw pointers for ownership
- **Memory**: Prefer RAII and smart pointers (`std::unique_ptr`, `std::shared_ptr`); apply `const` correctness
- **Enums**: Use `enum class` over unscoped enums
- **Test placement**: Tests live under `tests/sources/`; use `TEST_F` fixtures; link with `gtest_main`
- **Formatting**: Run `./scripts/run-clang-format.sh` before committing; 80-column limit, tabs for indentation

## CMake conventions for agents

- **Target-scoped commands only**:
  - Use `target_include_directories()`, `target_link_libraries()`, `target_compile_definitions()`, `target_compile_features()`
  - Avoid global `include_directories()`, `add_definitions()`, `add_compile_definitions()` (except in very limited cases like global GLM flags)
- **Adding new targets**:

  ```cmake
  add_library(${PROJECT_NAME} ${SOURCES})
  target_include_directories(${PROJECT_NAME}
      PUBLIC
          $<BUILD_INTERFACE:${HEADERS_DIR}>
          $<INSTALL_INTERFACE:include>
  )
  target_link_libraries(${PROJECT_NAME}
      PUBLIC
          dependency1
          dependency2
  )
  ```

- **Dependency management**:
  - Use `FetchContent_Declare()` + `FetchContent_MakeAvailable()` for external deps
  - Link dependencies per-target with `PUBLIC`/`PRIVATE` visibility
  - Set dependency options before `FetchContent_MakeAvailable()` (e.g., `set(GLM_BUILD_TESTS OFF)`)
- **Tests**: Use `enable_testing()`, `FetchContent` for GoogleTest, `gtest_discover_tests()` for auto-discovery
- **Options**: Use `option()` for user-configurable flags; validate mutually exclusive options at configure time
- **Platform/backend logic**: Guard platform-specific code with `if(BUILD_FOR_*)` checks; use `return()` to skip unrelated platform subdirs

## Editing boundaries

- **Do not edit**:
  - `build/` — auto-generated CMake output
  - Third-party code fetched via FetchContent (e.g., `build/_deps/`)
  - `.github/workflows/` — CI configuration (requires approval)
  - `cmake/Config*.cmake` — platform toolchain configs (requires approval)
- **Edit with care**:
  - Public headers in `headers/evan/` — update docs/tests if API changes
  - `CMakeLists.txt` files — ensure target-scoped commands, maintain option validation
  - `tests/` — keep tests in sync with module changes
- **Require human approval before**:
  - Changing C++ standard, CMake minimum version, or compiler flags
  - Adding new external dependencies or changing dependency versions
  - Modifying platform/backend option validation logic
  - Changing public API that affects ABI (virtual methods, class layouts)

## Change checklist

- [ ] Configure and build succeeds for at least one platform/backend combination
- [ ] Relevant tests pass (`ctest --output-on-failure`)
- [ ] Formatting applied via `./scripts/run-clang-format.sh`
- [ ] CMake updated consistently (target-scoped commands, new files added to `GLOB_RECURSE` or explicit lists)
- [ ] Docs/tests updated when behavior or API changes
- [ ] Namespace and naming conventions followed
- [ ] No global `include_directories()` or `add_definitions()` introduced

## Safe example tasks

1. **Add a unit test for an existing function**: Create a new `.cpp` file in `tests/sources/`, write `TEST_F` cases using existing fixtures, ensure `GLOB_RECURSE` picks it up, run `ctest`.

2. **Add a source/header pair to an existing module**: Place `.hpp` in `headers/evan/` and `.cpp` in `sources/`, follow naming conventions (`PascalCase` class, `camelCase` methods, `_` prefix for members), update namespace nesting, verify build.

3. **Refactor a component to match existing error handling patterns**: Replace raw pointers with `std::unique_ptr` or `std::shared_ptr`, add `const` correctness, use `enum class` for state machines, follow class layout order (public → protected → private).
