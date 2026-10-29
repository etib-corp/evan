# Testing & Coverage

Evan's tests use GoogleTest and are registered with CTest. They are headless:
no Vulkan device, window, or headset is required.

## Test tooling

- **GoogleTest** is the test framework. It provides fixtures (`TEST_F`), which
  the backend-abstraction tests rely on, and it is fetched through
  `FetchContent` from the vendored `third_party/googletest`, so tests do not
  depend on a system package or on the network.
- **CTest + `gtest_discover_tests()`**: the test executable is scanned at build
  time and every GoogleTest case is registered as its own CTest entry. This
  gives per-test CI reporting and `-R` filtering. Each case runs in a separate
  process, which matters for coverage (see the Windows and macOS sections).
- **One test executable**, `test_evan`, with an explicit source list (no
  `file(GLOB)`) in `tests/CMakeLists.txt`. It links the static `evan` library
  and `gtest_main`.
- Tests never create a Vulkan instance, window, or XR session; backend types are
  exercised through fake handles. They can therefore run on any desktop OS and
  in CI without a GPU or headset.
- `BUILD_TESTING` defaults to `ON` because the root `CMakeLists.txt` includes
  `CTest`; pass `-DBUILD_TESTING=OFF` to skip the tests.

## Running the tests

Enable `BUILD_TESTING` for one backend/platform pair:

```sh
cmake -S . -B build \
  -DBUILD_FOR_GLFW=ON \
  -DBUILD_FOR_LINUX=ON \
  -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Replace `BUILD_FOR_GLFW` with `BUILD_FOR_OPENXR`, and `BUILD_FOR_LINUX` with
`BUILD_FOR_MACOS` or `BUILD_FOR_WINDOWS`, to test another configuration.

Run a subset by name with `-R`:

```sh
ctest --test-dir build -R FrameSlots --output-on-failure
```

## Test matrix in CI

| Job                                       | Configurations                                                                 | What runs                     |
| ----------------------------------------- | ------------------------------------------------------------------------------ | ----------------------------- |
| `build` (`.github/workflows/build.yml`)   | GLFW Linux/macOS/Windows, OpenXR Linux, OpenXR Android (build only)            | `ctest` on every non-Android  |
| `sanitizers` (`sanitizers.yml`)           | GLFW Linux, GCC with AddressSanitizer + UndefinedBehaviorSanitizer             | `ctest`                       |
| `coverage` (`sanitizers.yml`)             | GLFW Linux, OpenXR Linux, GLFW macOS, GLFW Windows                             | `ctest` under a collector     |

The matrix is driven by what the codebase actually supports:

- **Android has no tests.** `build.yml` passes `BUILD_TESTING=OFF` for the
  Android job, and CI has no device or emulator to run an APK on. Android
  sources are therefore compile-checked only.
- **OpenXR is tested on Linux only.** The OpenXR backend has platform sources
  for Linux and Android only (`sources/openxr/platform/`); macOS and Windows
  OpenXR configurations are not part of the build matrix.
- **GLFW is tested on all three desktop OSes.** Each OS compiles a different
  platform file, so running all three is what covers the platform-specific
  code.
- **Sanitizers run on one representative configuration** (GLFW Linux, GCC
  `-fsanitize=address,undefined -fno-omit-frame-pointer`). The other OSes are
  covered by the build and coverage jobs; this keeps CI time bounded.

## Coverage

CI collects coverage for every configuration that builds the tests and uploads
the reports to Codecov (see the `coverage` job in
`.github/workflows/sanitizers.yml`):

| Backend | OS      | Tooling                              |
| ------- | ------- | ------------------------------------ |
| GLFW    | Linux   | GCC `--coverage` + `lcov`            |
| OpenXR  | Linux   | GCC `--coverage` + `lcov`            |
| GLFW    | macOS   | AppleClang profile + `llvm-cov`      |
| GLFW    | Windows | MSVC Debug + OpenCppCoverage         |

Codecov merges the per-OS uploads for a commit, so platform-specific source
files get covered by the OS that compiles them.

### Why these tools on each platform

There is no single portable C++ coverage collector, so each platform uses the
tool that requires the least additional toolchain:

- **Linux: GCC + lcov.** GCC is the default compiler on `ubuntu-latest` and the
  one used by the sanitizer job, so no extra compiler is installed. GCC's
  `--coverage` writes `.gcno`/`.gcda` data, and `lcov` is packaged by Ubuntu
  and exports the lcov format Codecov accepts. Clang + `llvm-cov` would work
  but would introduce a second compiler and a second toolchain to keep in
  sync.
- **macOS: AppleClang + llvm-cov.** Xcode ships LLVM's source-based coverage
  runtime and `xcrun llvm-profdata`/`llvm-cov`, but no `gcov`, and lcov is not
  installed on the macOS runner. `-fprofile-instr-generate
  -fcoverage-mapping` is the native Apple toolchain path and needs no external
  package. `llvm-cov export -format=lcov` converts the result to the same
  format as Linux, so Codecov normalizes both the same way.
- **Windows: OpenCppCoverage.** MSVC has no free standalone command-line
  coverage collector, and OpenCppCoverage is the only free tool that works
  directly on the MSVC build without changing the compiler. Details and the
  rejected alternatives are below.

### Why OpenCppCoverage on Windows

MSVC itself does not provide a coverage collector useful in CI, so the choice
is between changing toolchains and adding a third-party collector.
[OpenCppCoverage](https://github.com/OpenCppCoverage/OpenCppCoverage) was
chosen because it:

- is open source (GPL-3.0) and installable from Chocolatey, which
  `windows-latest` already provides (`choco install opencppcoverage`);
- reads the program database (`.pdb`) files at run time, so no compiler or
  linker flags are added and the coverage job builds the same MSVC Debug
  configuration as everyone else;
- supports optimized builds, so it does not force `Debug` when symbols are
  present;
- exports Cobertura XML directly, which Codecov accepts;
- can follow child processes with `--cover_children`, which is required
  because CTest launches the test executable as a child process.

Alternatives considered and rejected:

| Alternative                                              | Why it is not used                                                                                                                                                                                                 |
| -------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Visual Studio Enterprise code coverage (`CodeCoverage.exe`) | Only licensed with Visual Studio Enterprise; unavailable on Build Tools or self-hosted runners without Enterprise. It also emits the proprietary `.coverage` format, which needs a converter before Codecov. |
| clang-cl with `-fprofile-instr-generate` + `llvm-cov`    | Works on Windows, but the Windows build is validated with MSVC (`/W4 /permissive-`, `/MD`, `/Od /Zi`). A second compiler would test a toolchain that is not shipped and would need matched LLVM versions. |
| MinGW-w64 with GCC `--coverage` + lcov                    | Same objection: it changes the compiler, C++ runtime, and ABI of the Windows job away from MSVC.                                                                                                                    |
| Commercial tools (BullseyeCoverage, Parasoft C/C++test)  | Paid per-seat/CI licenses; unnecessary for a single test executable.                                                                                                                                                 |
| Codecov or another uploader on its own                   | It reports coverage; it does not collect it. A collector is still required on every platform.                                                                                                                       |

OpenCppCoverage needs PDB files. On Windows, CMake uses the Visual Studio
multi-config generator, and `cmake --build` builds the `Debug` configuration by
default; `ConfigWindows.cmake` adds `/Od /Zi` for Debug and CMake's Debug
linker flags add `/DEBUG`, so the PDB exists without extra options. With a
single-config generator (for example Ninja) or a `Release` build, provide
symbols explicitly with `/Zi` and `/DEBUG`.

### Linux (GCC + lcov)

```sh
cmake -S . -B /tmp/evan-coverage \
  -DBUILD_FOR_GLFW=ON \
  -DBUILD_FOR_LINUX=ON \
  -DBUILD_TESTING=ON \
  -DCMAKE_CXX_FLAGS=--coverage \
  -DCMAKE_EXE_LINKER_FLAGS=--coverage
cmake --build /tmp/evan-coverage -j
ctest --test-dir /tmp/evan-coverage --output-on-failure

lcov --capture --directory /tmp/evan-coverage \
  --output-file coverage.info \
  --no-external --ignore-errors unused \
  --exclude '*/_deps/*' \
  --exclude '*/tests/*' \
  --exclude '*/googletest*'
lcov --summary coverage.info
genhtml coverage.info --output-directory coverage-html
```

`--coverage` must be on both the compile and link command lines: it instruments
the static `evan` library at compile time and links `libgcov` into
`test_evan`. `--no-external` drops system headers; the excludes remove fetched
dependencies, the tests, and GoogleTest. `--ignore-errors unused` is needed
because lcov 2.x treats an exclude pattern that matched nothing as an error.

### macOS (AppleClang)

Build with source-based coverage in a scratch directory:

```sh
cmake -S . -B /tmp/evan-coverage \
  -DBUILD_FOR_GLFW=ON \
  -DBUILD_FOR_MACOS=ON \
  -DBUILD_TESTING=ON \
  '-DCMAKE_CXX_FLAGS=-fprofile-instr-generate -fcoverage-mapping' \
  '-DCMAKE_EXE_LINKER_FLAGS=-fprofile-instr-generate'
cmake --build /tmp/evan-coverage -j 8
```

`-fcoverage-mapping` embeds the source map, and `-fprofile-instr-generate`
links the profiling runtime. Run the tests, writing one profile per test
process:

```sh
LLVM_PROFILE_FILE=/tmp/evan-coverage/coverage-%p.profraw \
  ctest --test-dir /tmp/evan-coverage --output-on-failure
xcrun llvm-profdata merge -sparse /tmp/evan-coverage/coverage-*.profraw \
  -o /tmp/evan-coverage/coverage.profdata
```

The `%p` in `LLVM_PROFILE_FILE` is required: `gtest_discover_tests` starts one
process per test case, and each process writes its own profile. `llvm-profdata
merge` combines them.

Print a per-file summary:

```sh
xcrun llvm-cov report /tmp/evan-coverage/tests/test_evan \
  -instr-profile=/tmp/evan-coverage/coverage.profdata \
  -ignore-filename-regex='.*/(third_party|_deps|tests|examples)/.*'
```

Or browse the annotated sources:

```sh
xcrun llvm-cov show /tmp/evan-coverage/tests/test_evan \
  -instr-profile=/tmp/evan-coverage/coverage.profdata \
  -ignore-filename-regex='.*/(third_party|_deps|tests|examples)/.*' \
  -format=html -output-dir=/tmp/evan-coverage/html
open /tmp/evan-coverage/html/index.html
```

Export the lcov file uploaded by CI:

```sh
xcrun llvm-cov export /tmp/evan-coverage/tests/test_evan \
  -instr-profile=/tmp/evan-coverage/coverage.profdata \
  -format=lcov \
  -ignore-filename-regex='.*/(third_party|_deps|tests|examples)/.*' \
  > /tmp/evan-coverage/coverage.info
```

The `llvm-*` tools must come from the same Xcode toolchain that compiled the
test executable; use `xcrun` rather than another LLVM installation.

### Windows (MSVC + OpenCppCoverage)

OpenCppCoverage instruments the Debug build at run time, so no extra compiler
flags are needed:

```powershell
choco install -y opencppcoverage
cmake -S . -B build -DBUILD_FOR_GLFW=ON -DBUILD_FOR_WINDOWS=ON -DBUILD_TESTING=ON
cmake --build build
OpenCppCoverage --sources "$PWD\sources" `
  --cover_children `
  --export_type cobertura:"$PWD\coverage.xml" `
  -- ctest --test-dir "$PWD\build" --output-on-failure
```

`--cover_children` is mandatory here: `ctest` launches `test_evan` as a child
process, and OpenCppCoverage covers only the process it starts unless told
otherwise. Without the flag, the report contains `ctest` only, which has no
Evan sources. `--sources "$PWD\sources"` limits instrumentation to the engine
implementation; public headers are not counted by the Windows job (the Linux
and macOS jobs do count them).

`coverage.xml` uses the Cobertura format accepted by Codecov. CTest uses the
`Debug` configuration by default for the Visual Studio generator; pass
`-C Debug` to `ctest` if a different configuration is selected explicitly.

### Merging and reporting

- Each of the four coverage jobs uploads its own report with a distinct
  `name`. Codecov merges all uploads for a commit, so a line counts as covered
  if any configuration executes it. Files that only exist on one platform (for
  example `MacOsDesktopPlatform.cpp`) come from that platform's upload.
- Linux and macOS upload lcov files; Windows uploads Cobertura XML. Codecov
  ingests both formats natively, so no conversion step is needed.
- `fail_ci_if_error: false` is set on the upload step: a Codecov outage or
  token problem does not fail the build. Coverage is reported, not gated;
  there is no minimum coverage threshold.
- Exclusions differ slightly by platform. Linux and macOS exclude
  `third_party/`, fetched dependencies (`_deps`), and the tests themselves;
  Windows limits instrumentation to `sources/` with `--sources`.

### Reading the report

The filters above exclude `third_party/`, fetched dependencies (`_deps`), and
the tests themselves. On Linux and macOS public headers under `headers/` are
still counted, so a change that adds header-only code moves the total even
when `sources/` is unchanged. On Windows only files under `sources/` are
instrumented.
