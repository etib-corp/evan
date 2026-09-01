# Evan Examples

This directory contains runnable samples that demonstrate how to use the Evan
framework.

## scene_objects

A minimal sample that demonstrates the CPU-side `Scene` object-management API
(adding and removing renderable objects). It does not require a live Vulkan
device, so it builds and runs anywhere.

### Building

```sh
cmake -S . -B build -DBUILD_EXAMPLES=ON
cmake --build build --target evan_scene_objects
```

### Running

```sh
./build/examples/scene_objects/evan_scene_objects
```
