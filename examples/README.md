# Evan Examples

This directory contains runnable samples that demonstrate how to use the Evan
framework.

## GLFW-linux-base

A minimal GLFW desktop sample that runs on Linux. It creates a
`LinuxDesktopPlatform`, draws one cube and a "Hello World!" text object, and
drives the `Engine` update/render/poll loop.

### Building

```sh
cmake -S . -B build \
  -DBUILD_FOR_GLFW=ON \
  -DBUILD_FOR_LINUX=ON \
  -DBUILD_EXAMPLES=ON
cmake --build build --target GLFW-linux-base
```

### Running

```sh
./build/examples/GLFW-linux-base/GLFW-linux-base
```

## GLFW-macos-base

A minimal GLFW desktop sample that runs on macOS (Vulkan through MoltenVK). It
creates a `MacOsDesktopPlatform` and drives the `Engine` update/render/poll
loop on an empty scene.

### Building

```sh
cmake -S . -B build \
  -DBUILD_FOR_GLFW=ON \
  -DBUILD_FOR_MACOS=ON \
  -DBUILD_EXAMPLES=ON
cmake --build build --target GLFW-macos-base
```

### Running

```sh
./build/examples/GLFW-macos-base/GLFW-macos-base
```

## GLFW-macos-poc

The P3 proof-of-concept workload used to fill the trigger table of
`docs/DESIGN_P3_OPTIMIZATIONS.md` §4. It is the same GLFW/macOS sample as
`GLFW-macos-base` plus an N-object scene driven by two knobs, both `0` by
default:

| Variable | Default | Meaning |
| -------- | ------- | ------- |
| `EVAN_POC_MESHES` | `0` | Cubes to spawn: one shared local-space cube mesh, one `Primitive` per object, laid out on a grid in front of the camera with `Engine::setObjectTransform`. |
| `EVAN_POC_FRAMES` | `0` | Stop after that many frames and exit. |

### Building

```sh
cmake -S . -B build \
  -DBUILD_FOR_GLFW=ON \
  -DBUILD_FOR_MACOS=ON \
  -DBUILD_EXAMPLES=ON
cmake --build build --target GLFW-macos-poc
```

### Running

```sh
cd build/examples/GLFW-macos-poc
EVAN_STATS=1 EVAN_STATS_INTERVAL=60 \
  EVAN_POC_MESHES=1000 EVAN_POC_FRAMES=420 \
  UTILITY_LOG_LEVEL=info NO_COLOR=1 ./GLFW-macos-poc
```

`scripts/run-poc.sh` wraps this for N=100 and N=1000, averages the report
windows and writes a markdown table; see `docs/PROFILING.md` §6.

## GLFW-linux-poc

The same P3 proof-of-concept workload as `GLFW-macos-poc`, on the Linux
backend. It takes the same `EVAN_POC_MESHES` / `EVAN_POC_FRAMES` knobs.

### Building

```sh
cmake -S . -B build \
  -DBUILD_FOR_GLFW=ON \
  -DBUILD_FOR_LINUX=ON \
  -DBUILD_EXAMPLES=ON
cmake --build build --target GLFW-linux-poc
```

### Running

```sh
cd build/examples/GLFW-linux-poc
EVAN_STATS=1 EVAN_STATS_INTERVAL=60 \
  EVAN_POC_MESHES=1000 EVAN_POC_FRAMES=420 \
  UTILITY_LOG_LEVEL=info NO_COLOR=1 ./GLFW-linux-poc
```

## GLFW-stress

The stress harness behind `scripts/run-stress.sh` and
`docs/STRESS_TESTING.md`. It measures one configuration and prints a
machine-readable `evan stress:` record (frame rate, frame-time percentiles,
draw calls, pipeline/descriptor binds, culling, CPU phases, GPU pass time).
It builds on macOS, Linux and Windows from the same source.

| Flag | Default | Meaning |
| ---- | ------- | ------- |
| `--meshes N` | `1000` | Cubes to spawn: one `GPUMesh` and one draw call each, on a grid fitted to the view. |
| `--pipelines K` | `1` | Distinct pipelines the scene draws through. The harness registers K-1 extra shader modules (copies of the mesh SPIR-V) and cycles the objects over them. |
| `--frames F` | `300` | Measured frames. |
| `--warmup W` | `60` | Warm-up frames, excluded from the record. |
| `--instancing on\|off`, `--indirect on\|off`, `--culling on\|off` | `off`/`off`/`on` | Renderer batching and culling switches. |
| `--present-mode M` | `fifo` | `fifo`, `mailbox`, `immediate` or `auto` (`EVAN_PRESENT_MODE`); uncapped modes are what a throughput benchmark needs. |
| `--csv FILE` | unset | Append one CSV row per run. |

### Building

```sh
cmake -S . -B build \
  -DBUILD_FOR_GLFW=ON \
  -DBUILD_FOR_MACOS=ON \
  -DBUILD_EXAMPLES=ON
cmake --build build --target GLFW-stress
```

### Running

```sh
cd build/examples/GLFW-stress
NO_COLOR=1 UTILITY_LOG_LEVEL=warning ./GLFW-stress \
  --meshes 2000 --pipelines 4 --frames 240 --warmup 60 \
  --present-mode mailbox
```

`scripts/run-stress.sh` sweeps this example and solves the mesh and pipeline
limits for a set of frame-rate thresholds; see `docs/STRESS_TESTING.md`.

## GLFW-windows-base

A minimal GLFW desktop sample that runs on Windows. It creates a
`WindowsDesktopPlatform` and drives the `Engine` update/render/poll loop.

### Building

```sh
cmake -S . -B build \
  -DBUILD_FOR_GLFW=ON \
  -DBUILD_FOR_WINDOWS=ON \
  -DBUILD_EXAMPLES=ON
cmake --build build --target GLFW-windows-base
```

### Running

```sh
./build/examples/GLFW-windows-base/GLFW-windows-base
```

## OpenXR-linux-base

A minimal OpenXR sample that runs on Linux desktop. It creates a
`LinuxXrPlatform` and drives the `Engine` update/render/poll loop. It requires
an OpenXR runtime (e.g. Monado) and a Vulkan-capable device.

### Building

```sh
cmake -S . -B build \
  -DBUILD_FOR_OPENXR=ON \
  -DBUILD_FOR_LINUX=ON \
  -DBUILD_EXAMPLES=ON
cmake --build build --target OpenXR-linux-base
```

### Running

```sh
./build/examples/OpenXR-linux-base/OpenXR-linux-base
```

## OpenXR-android-base

A full Android (Gradle/NDK) OpenXR sample. It uses `AndroidXrPlatform` with the
native-app-glue lifecycle handling and drives the `Engine` loop. It is built
with Gradle, not the workspace CMake.

### Building

```sh
cd examples/OpenXR-android-base
./gradlew assembleDebug
```

The resulting APK is written to
`app/build/outputs/apk/debug/app-debug.apk` and can be installed on a
headset/device with `adb install`.

