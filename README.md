# Evan

Evan is a component-based UI framework library built around a primitive rendering system. It's designed for building user interfaces across XR platforms and traditional platforms with custom rendering backends and input handling. The framework is lightweight, flexible, and easy to integrate into existing projects.

## Key Features

- **Primitive-Based Rendering**: Components generate rendering primitives, enabling clean separation between UI logic and visual output
- **Flexible Renderer Architecture**: Support for multiple rendering backends (terminal, GUI, web, graphics APIs)
- **Component Hierarchy**: Tree-structured components with state management and event handling
- **Reactive Updates**: Event-driven state changes with explicit re-rendering
- **Extensible Primitive System**: Built-in primitives (Text, Rectangle, Polygon) with support for custom primitives

## Quick Start

```cpp
#include "application.hpp"
#include "label.hpp"
#include "button.hpp"

// Create custom renderer
class MyRenderer : public Renderer {
public:
    void draw(std::shared_ptr<Primitive> primitive) override {
        if (auto text = std::dynamic_pointer_cast<Text>(primitive)) {
            std::cout << "Text: " << text->getContent() << std::endl;
        }
        // Handle other primitive types...
    }
};

int main() {
    // Create application with renderer
    Application app(std::make_unique<MyRenderer>());

    // Build UI
    auto root = app.getRoot();
    root->addChild(std::make_shared<Label>("Hello Evan!"));

    // Run
    app.run();
    return 0;
}
```

## Documentation

- **[How Evan Works](docs/HOW_GUILLAUME_WORKS.md)** - Comprehensive guide to concepts, lifecycle, and usage patterns
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
