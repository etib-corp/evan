/*
** ETIB PROJECT, 2026
** evan
** File description:
** Software
*/

#pragma once

#include "Version.hpp"

#include "DeviceContext.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "ASwapchainContext.hpp"
#include "IPlatform.hpp"

#ifdef __OPENXR__
    #include "openxr/XRPlatform.hpp"
    #include "openxr/XRSwapchainContext.hpp"
#elif defined(__GLFW__)
    #include "glfw/DesktopPlatform.hpp"
    #include "glfw/DesktopSwapchainContext.hpp"
#else
    #error "Unsupported platform"
#endif

#include <string>
#include <memory>

namespace evan
{
    /**
     * @class Software
     * @brief Represents the software information of the engine, including its name and version.
     *
     * The Software class encapsulates details about the engine's software, such as its name and version. It provides a structured way to access this information throughout the engine.
     *
     * @var Software::_engineName
     *      The name of the engine.
     * @var Software::_engineVersion
     *      The version of the engine as a string.
     * @var Software::_version
     *      The version of the engine as a Version object.
     */
    class Software {
        public:
            Software();
            Software(const std::string &windowName, const uint32_t width, const uint32_t height);
            ~Software();

            void init();

            void run();

            void update(); // For logic updates, input handling, etc.

            void render(); // For rendering the current scene.

            void addScene(std::vector<std::string> texturePaths, std::map<std::string, std::vector<Mesh>> meshData);

        protected:
            const std::string _engineName = "Evan Engine";                                  /// The name of the engine.
            const std::string _engineVersion = "0.1.0";                                     /// The version of the engine as a string.
            const std::shared_ptr<Version> _version = std::make_shared<Version>(0, 1, 0);   /// The version of the engine as a Version object.

            std::shared_ptr<DeviceContext> _deviceContext;  /// A shared pointer to a DeviceContext object, which manages the Vulkan device and related resources. The DeviceContext is responsible for initializing Vulkan, selecting physical devices, creating logical devices, and managing command pools and queues.
            std::shared_ptr<Renderer> _renderer;    /// A shared pointer to a Renderer object, which is responsible for rendering scenes. The Renderer manages graphics pipelines, descriptor sets, and other rendering resources needed to draw objects on the screen.
            std::vector<Scene> _scenes;    /// A vector of Scene objects, representing the different scenes that can be rendered or managed by the engine. Each Scene contains its own set of objects, materials, and other relevant data for rendering.
            uint32_t _currentScene;    /// An index to keep track of the current scene being rendered or managed. This allows the engine to switch between different scenes as needed.
            std::shared_ptr<ASwapchainContext> _swapchainContext;  /// A shared pointer to an ASwapchainContext object, which manages the Vulkan swapchain and related resources. The ASwapchainContext is responsible for creating and managing the swapchain, handling presentation, and managing synchronization primitives related to rendering and presentation.
            std::shared_ptr<IPlatform> _platform;  /// A shared pointer to an IPlatform object, which provides an abstraction layer for platform-specific operations. The IPlatform interface defines methods for window management, input handling, and other platform-dependent functionality, allowing the engine to be portable across different operating systems and platforms.
        private:
    };
} // namespace evan

