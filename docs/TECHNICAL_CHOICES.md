# Technical Choices

This document outlines the key technical choices made in the development of this project, including programming languages, libraries, and tools used.

## Programming Languages

We have chosen `C++20` as the **primary** programming language for this project due to its performance, efficiency, and control over system resources. C++ is well-suited for applications that require high performance and low-level memory management.

We could have chosen using *Java* (regarding technologies we have chosen) but it is less efficient and only android devices oriented, but we want to target more different type of devices.

With this programing language, we have chosen these libraries.

### Shaders format

We use `glsl`, which is used by most of open-source projects.

### Scripting

We surely will have to develop a scripting language further.

## Libraries

### Vulkan

We use the Vulkan API for graphics rendering. Vulkan is a low-level, cross-platform graphics API that provides high performance and flexibility for rendering 2D and 3D graphics. It allows us to have fine-grained control over the GPU, which is essential for achieving optimal performance in graphics-intensive applications.

And why over other graphics APIs like OpenGL or DirectX:

- **Performance**: Vulkan offers better performance and lower CPU usage compared to OpenGL, making it ideal for high-performance applications.

- **Multi-threading**: Vulkan is designed to take advantage of multi-core processors, allowing for better parallelism and improved performance in multi-threaded applications.

|           | NVIDIA RTX 4090 | AMD RX 7900 XTX | Intel Arc A770 |
|-----------|:---------------:|:---------------:|:--------------:|
| Vulkan    |     130 FPS     |     125 FPS     |     85 FPS     |
| DirectX12 |     125 FPS     |     120 FPS     |     80 FPS     |
| OpenGL    |     90 FPS      |     85 FPS      |     60 FPS     |

> Here we can see in gross performances that Vulkan is more performant

|           | Intel i9-13900K | AMD Ryzen 9 7950X |
|-----------|:---------------:|:-----------------:|
| Vulkan    |        25%      |        22%        |
| DirectX12 |        30%      |        28%        |
| OpenGL    |        55%      |        50%        |

> Here we can see in CPU usage that Vulkan uses less than other APIs

|           | Multithreading handling |          How much ?           |
|-----------|:-----------------------:|:-----------------------------:|
| Vulkan    |        Optimized        | Most performant but complex   |
| DirectX12 |        Optimized        | CPU threads and GPU's command |
| OpenGL    |    No (Limitations)     | Only one thread can handle    |

> Here Vulkan and DirectX are equals, and at this point OpenGL is out of the race

|         | Windows | Linux | macOS | Android | iOS |
|---------|:-------:|:-----:|:-----:|:-------:|:---:|
| Vulkan  |    ✅   |   ✅   |   ⚠️  |    ✅   |  ⚠️  |
| DirectX |    ✅   |   ❌   |   ❌  |    ❌   |  ❌  |
| OpenGL  |	   ✅   |   ✅   |   ✅  |    ✅   |  ✅  |

> Clearly DirectX is made only for Windows. So we can not use it for the project. Maybe for an optimized Windows version further ?


*See: [malekal (in french)](https://www.malekal.com/directx-vs-opengl-vs-vulkan-lequel-est-le-meilleur/), [windowsreport (comparison between VK and DX12)](https://windowsreport.com/directx-12-vs-vulkan-a-detailed-comparison/)*

### OpenXR

We use OpenXR for handling virtual reality (VR) and augmented reality (AR) experiences. OpenXR is an open standard that provides a unified interface for VR and AR applications, allowing us to support multiple hardware platforms with a single codebase.

OpenXR was chosen over other VR/AR frameworks for the following reasons:

- **Cross-platform support**: OpenXR supports a wide range of VR and AR devices, allowing us to reach a broader audience without needing to develop separate codebases for each platform.

- **Future-proofing**: As an open standard, OpenXR is designed to evolve with the VR/AR industry, ensuring that our application remains compatible with future devices and technologies.

OpenXR is also based on Vulkan, so it will make the development easier.

We could have chosen other APIs such as:

- Meta XR SDK
- Windows Mixed Reality (WMR)
- Google ARCore
- Apple ARKit

But most of these APIs are migrating to OpenXR.
And the point with ARCore is that Google use it in its new AndroidXR and we want to make something different, so using it would be a paradox.

We could also note that WMR is not possible, because it is made for Windows / Microsoft devices (same for Meta XR SDK)

### GLFW

We use GLFW as a library for creating windows, contexts, and handling input for our application. GLFW is a lightweight, cross-platform library that simplifies the process of creating and managing OpenGL and Vulkan contexts.

GLFW was chosen, over SDL for the following reasons:

- **Simplicity**: GLFW has a simple and straightforward API, making it easy to integrate into our application without unnecessary complexity.

- **Focus on graphics**: GLFW is specifically designed for graphics applications, providing the necessary functionality without additional features that may not be needed for our project.

But we are considering moving soon to SDL3, because of its high compatibility, event management, ...

## Build System

We have chosen CMake as our build system.
Inside the CMake, we have the possibility to build two different versions of the project:

- **GLFW**: This version uses the GLFW library for window management and input handling.
- **OpenXR**: This version uses the OpenXR library for VR/AR support.

Then, depending on the user's needs, they can choose which version to build and use.

