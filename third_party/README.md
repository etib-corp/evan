# Third-party dependencies

This directory contains vendored copies of all third-party dependencies used by
Evan. They are consumed at configure time via `FetchContent` with a local
`SOURCE_DIR`, so no network access is required to build the project.

Each dependency is pinned to a specific upstream version. To update a
dependency, replace the contents of its directory with the new upstream release
and update the version recorded below.

| Directory              | Dependency          | Version / commit                          | Upstream                                                              | License |
| ---------------------- | ------------------- | ----------------------------------------- | --------------------------------------------------------------------- | ------- |
| `glm/`                 | GLM                 | `1.0.2`                                   | https://github.com/g-truc/glm                                         | MIT (Happy Bunny / MIT) |
| `stb/`                 | stb                 | `2c980bb59875b0d32144a71867fbdebb2f77cd20` | https://github.com/nothings/stb                                       | MIT / Public Domain |
| `glfw/`                | GLFW                | `3.4`                                     | https://github.com/glfw/glfw                                          | Zlib |
| `OpenXR-SDK/`          | OpenXR SDK          | `release-1.1.54`                          | https://github.com/KhronosGroup/OpenXR-SDK-Source                      | Apache-2.0 |
| `googletest/`          | GoogleTest          | `v1.17.0`                                 | https://github.com/google/googletest                                  | BSD-3-Clause |
| `googlebenchmark/`     | Google Benchmark    | `v1.9.1`                                  | https://github.com/google/benchmark                                   | Apache-2.0 |
| `doxygen-awesome-css/` | Doxygen Awesome CSS | `v2.4.1`                                  | https://github.com/jothepro/doxygen-awesome-css                       | MIT |

## Notes

- The vendored copies are stripped of their `.git` directories and
  `.gitmodules` files to keep the repository lean.
- Each dependency retains its own license file; refer to the individual
  `LICENSE`/`COPYING` files for the exact terms.
- The `etib-corp` sibling library (`utility`) is intentionally not vendored; it
  is fetched via `FetchContent` with a local-checkout override for development.
