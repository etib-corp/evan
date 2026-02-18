/*
** ETIB PROJECT, 2026
** evan
** File description:
** DeviceContext
*/

#pragma once

#include "EvanPlatform.hpp"

namespace evan
{
     /**
      * @class DeviceContext
      * @brief Represents the device context for the engine, managing Vulkan-related resources and operations.
      *
      * The DeviceContext class encapsulates the Vulkan instance, physical device, logical device, graphics queue, command pool, and MSAA samples. It provides a structured way to manage these resources and perform operations related to Vulkan rendering.
      *
      */
    class DeviceContext {
        public:
            DeviceContext();
            ~DeviceContext();

        protected:
            VkInstance _VkInstance;    /// The Vulkan instance, which is the connection between the application and the Vulkan library. It is used to create and manage Vulkan resources and to query the capabilities of the Vulkan implementation.
            VkPhysicalDevice _physicalDevice;    /// The physical device, which represents a Vulkan-compatible GPU. It is used to query the capabilities of the GPU and to create logical devices for rendering operations.
            VkDevice _device;    /// The logical device, which represents an instance of a physical device and is used to perform rendering operations.
            VkQueue _graphicsQueue;    /// The graphics queue, which is used to submit rendering commands to the GPU.
            VkCommandPool _commandPool;    /// The command pool, which is used to allocate command buffers for recording rendering commands.
            VkSampleCountFlagBits _msaaSamples;    /// The number of samples used for multisampling anti-aliasing (MSAA), which is a technique used to improve the visual quality of rendered images by reducing aliasing artifacts.
        private:
    };
} // namespace evan
