# Evan
Evan is a library for 3D rendering and input handling for applications on XR devices and traditional platforms. It is built on top of OpenXR and Vulkan, providing a high-level interface for developers to create immersive experiences.

## Key Features

- **3D Primitives**: Built-in support for rendering common 3D shapes like cubes, spheres, and text.
- **Event Handling**: Comprehensive event system for handling user input and interactions.

## Quick Start

```cpp
int main() {
    return 0;
}
```

## Documentation

- **[How Evan Works](docs/HOW_EVAN_WORKS.md)** - Comprehensive guide to concepts, lifecycle, and usage patterns
- **[Architecture Documentation](docs/ARCHITECTURE.md)** - Detailed overview of the system's design and components
- **[Technical Choices Documentation](docs/TECHNICAL_CHOICES.md)** - Insights into design decisions and technologies

## Building

Evan builds as a static library along with examples and tests:

```bash
mkdir build && cd build
cmake ..
make
```

### Running Examples

After building, you can run the examples:

```bash
# Hello World (minimal)
./examples/hello_world/hello_world

# Button Counter (interactive)
./examples/button_counter/button_counter

# Basic Demo (comprehensive)
./examples/basic_demo/basic_demo
```

### Running Tests

To run the test suite:

```bash
./tests/test_evan
```

### Using Evan in Your Project

Evan builds as a static library (`libevan.a`) with minimal dependencies. To use it in your CMake project:

```cmake
# Link with Evan (core library has no external dependencies)
target_link_libraries(your_target PRIVATE evan)

# Add SDL3 and OpenGL only if you need them for your custom renderer
# target_link_libraries(your_target PRIVATE SDL3::SDL3 ${OPENGL_LIBRARIES})
```

## Examples

The `examples/` directory contains sample applications demonstrating Evan's features. Each example is organized in its own subdirectory:

- **hello_world/**: Minimal example showing basic Evan usage
- **button_counter/**: Interactive example demonstrating event handling and state management
- **basic_demo/**: Comprehensive example showing component hierarchy, event handling, and 3D primitive rendering
