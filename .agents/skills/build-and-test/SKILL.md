---
name: build-and-test
description: Configure, build, test and format evan. Use after any change to headers/, sources/, tests/ or the CMake files, and whenever asked to build, test or check the library.
---

# Build and test evan

evan builds exactly one platform and one backend per build directory. Pick
the platform of the machine and the GLFW backend unless the task is about
OpenXR.

| Host | Platform option |
| --- | --- |
| macOS | `-DBUILD_FOR_MACOS=ON` |
| Linux | `-DBUILD_FOR_LINUX=ON` |
| Windows | `-DBUILD_FOR_WINDOWS=ON` |

```sh
cmake -S . -B build -DBUILD_FOR_GLFW=ON -DBUILD_FOR_<PLATFORM>=ON \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build -j 2>&1 | grep -E "error|Error [0-9]|Built target"
ctest --test-dir build --output-on-failure
```

- The Vulkan SDK must be installed; `find_package(Vulkan)` fails otherwise.
- The first configure fetches `utility` from Git and needs network access.
- The tests are headless: no window, GPU or headset. They run anywhere.
- Run one group with `ctest --test-dir build -R <Name> --output-on-failure`.
- A change under `sources/openxr/` or `headers/evan/openxr/` is only compiled
  with `-DBUILD_FOR_OPENXR=ON` (use a second directory, `build-openxr`). Say
  so when that configuration was not built.
- Examples are built with `-DBUILD_EXAMPLES=ON` and open a window.

## Format

```sh
./scripts/run-clang-format.sh
```

CI checks formatting with clang-format **21.1.8**; another version formats
differently. Install it with `pip install 'clang-format==21.1.8'`.

## Report

State the platform and backend that were built, the test count, and every
configuration the change touches that was not built. See the change checklist
in `AGENTS.md`. More detail: `docs/TESTING.md`.
