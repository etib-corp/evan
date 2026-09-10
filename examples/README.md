# Evan Examples

This directory contains runnable samples that demonstrate how to use the Evan
framework.

## GLFW-linux-base

A minimal GLFW desktop sample that runs on Linux. It creates a
`LinuxDesktopPlatform` and drives the `Engine` update/render/poll loop.

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

