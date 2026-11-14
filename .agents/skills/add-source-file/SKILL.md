---
name: add-source-file
description: Add, rename or remove a header, source or test file in evan. Use whenever a new class, module or test file is created, because source and test lists are explicit and a forgotten entry breaks the build or silently skips tests.
---

# Add a file to evan

Nothing is globbed. Every file is listed by hand.

## Class

1. Header: `headers/evan/<Name>.hpp` (backend code under
   `headers/evan/glfw/` or `headers/evan/openxr/`).
2. Source: `sources/<Name>.cpp`, same sub-directory as the header.
3. Add the `.cpp` to the right list in `cmake/Sources.cmake`:
   `EVAN_SOURCES_COMMON`, `EVAN_SOURCES_GLFW`, `EVAN_SOURCES_OPENXR` or
   `EVAN_SOURCES_OPENXR_ANDROID`. A file in the wrong list is compiled for the
   wrong backend.
4. Follow a neighbouring file for the license header, namespace, Doxygen
   comments and class layout (public, protected, private; `_` prefix on
   members).

## Test

1. `tests/sources/test_<name>.cpp`, with `TEST_F` fixtures. Tests must stay
   headless: use fake handles, never create a Vulkan instance or a window.
2. Add it to `TEST_SOURCES` in `tests/CMakeLists.txt`. A test file that is not
   listed is not compiled, and nothing warns about it.
3. Shared fixtures go in `tests/headers/`.

## Finish

Follow the `build-and-test` skill. Update `docs/` and `CHANGELOG.md` when a
public header changes. A new external dependency, a change to
`cmake/Config*.cmake` or to `.github/workflows/` needs human approval first
(see `AGENTS.md`, "Editing boundaries").
