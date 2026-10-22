# Testing & Coverage

Evan's tests use GoogleTest and are registered with CTest. They are headless:
no Vulkan device, window, or headset is required.

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

Android is not covered: tests are not built for Android and CI has no device or
emulator to run them on.

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

Run the tests, writing one profile per test process:

```sh
LLVM_PROFILE_FILE=/tmp/evan-coverage/coverage-%p.profraw \
  ctest --test-dir /tmp/evan-coverage --output-on-failure
xcrun llvm-profdata merge -sparse /tmp/evan-coverage/coverage-*.profraw \
  -o /tmp/evan-coverage/coverage.profdata
```

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

### Windows (MSVC + OpenCppCoverage)

OpenCppCoverage instruments the Debug build, so no extra compiler flags are
needed:

```powershell
choco install -y opencppcoverage
cmake -S . -B build -DBUILD_FOR_GLFW=ON -DBUILD_FOR_WINDOWS=ON -DBUILD_TESTING=ON
cmake --build build
OpenCppCoverage --sources "$PWD\sources" `
  --export_type cobertura:"$PWD\coverage.xml" `
  -- ctest --test-dir "$PWD\build" --output-on-failure
```

`coverage.xml` uses the Cobertura format accepted by Codecov.

### Reading the report

The filters above exclude `third_party/`, fetched dependencies (`_deps`), and
the tests themselves. Public headers are still counted, so a change that adds
header-only code moves the total even when `sources/` is unchanged.
