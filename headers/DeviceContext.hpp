/*
** ETIB PROJECT, 2026
** evan
** File description:
** DeviceContext
*/

#pragma once

#include "EvanPlatform.hpp"
#include "IDeviceBackend.hpp"

#include <iostream>
#include <memory>

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

            /**
             * @brief Retrieves the number of MSAA samples used by this device context.
             * 
             * @return VkSampleCountFlagBits The MSAA sample count flag bits representing the 
             *         number of samples per pixel for multisample anti-aliasing operations.
             */
            VkSampleCountFlagBits getMsaaSamples() const;  

        protected:
            VkInstance _VkInstance;    /// The Vulkan instance, which is the connection between the application and the Vulkan library. It is used to create and manage Vulkan resources and to query the capabilities of the Vulkan implementation.
            VkPhysicalDevice _physicalDevice;    /// The physical device, which represents a Vulkan-compatible GPU. It is used to query the capabilities of the GPU and to create logical devices for rendering operations.
            VkDevice _device;    /// The logical device, which represents an instance of a physical device and is used to perform rendering operations.
            VkQueue _graphicsQueue;    /// The graphics queue, which is used to submit rendering commands to the GPU.
            VkCommandPool _commandPool;    /// The command pool, which is used to allocate command buffers for recording rendering commands.
            VkSampleCountFlagBits _msaaSamples;    /// The number of samples used for multisampling anti-aliasing (MSAA), which is a technique used to improve the visual quality of rendered images by reducing aliasing artifacts.

            std::unique_ptr<IDeviceBackend> _deviceBackend;    /// A unique pointer to an IDeviceBackend instance, which is responsible for providing platform-specific implementations of Vulkan device management operations. The IDeviceBackend interface defines methods for creating Vulkan instances, logical devices, and picking physical devices, allowing for abstraction and flexibility in the device context implementation.
        private:

            /**
             * @brief Determines and sets the maximum usable sample count for multi-sample anti-aliasing (MSAA).
             * 
             * Queries the physical device properties to find the maximum sample count supported by both
             * the framebuffer color and depth attachments. Sets _msaaSamples to the highest supported
             * sample count, with a fallback to VK_SAMPLE_COUNT_1_BIT if no higher counts are available.
             * 
             * Supported sample counts in descending order of preference:
             * - VK_SAMPLE_COUNT_64_BIT
             * - VK_SAMPLE_COUNT_32_BIT
             * - VK_SAMPLE_COUNT_16_BIT
             * - VK_SAMPLE_COUNT_8_BIT
             * - VK_SAMPLE_COUNT_4_BIT
             * - VK_SAMPLE_COUNT_2_BIT
             * - VK_SAMPLE_COUNT_1_BIT (default)
             * 
             * @note This function should be called after the physical device has been selected.
             * @note The _msaaSamples member variable is updated with the determined sample count.
             */
            void getMaxUsableSampleCount();

            /**
             * @brief Creates a Vulkan command pool for the device context.
             * 
             * Initializes a command pool with the specified queue family index and flags.
             * The command pool is configured to allow individual command buffers to be reset.
             * 
             * @note The queue family index is currently hardcoded to 0, assuming the graphics
             *       queue family is at that index. This should be replaced with dynamic queue
             *       family selection in production code.
             * 
             * @see vkCreateCommandPool
             * @see VkCommandPoolCreateInfo
             */
            void createCommandPool();
    };
} // namespace evan
