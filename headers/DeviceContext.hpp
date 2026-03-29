/*
** ETIB PROJECT, 2026
** evan
** File description:
** DeviceContext
*/

#pragma once

#include "EvanPlatform.hpp"
#include "ADeviceBackend.hpp"

#include "IPlatform.hpp"

#include <iostream>
#include <memory>

namespace evan
{
	/**
	 * @class DeviceContext
	 *
	 * @brief Represents the device context for the engine, managing
	 * Vulkan-related resources and operations.
	 *
	 * The DeviceContext class encapsulates the Vulkan instance, physical
	 * device, logical device, graphics queue, command pool, and MSAA samples.
	 * It provides a structured way to manage these resources and perform
	 * operations related to Vulkan rendering.
	 */
	class DeviceContext
	{
		public:
		/**
		 * @brief Constructs a DeviceContext instance using the provided
		 * platform.
		 *
		 * This constructor initializes the device context by creating a Vulkan
		 * instance, selecting a physical device, creating a logical device, and
		 * setting up the graphics queue and command pool. It also determines
		 * the maximum usable sample count for MSAA and sets up the Vulkan debug
		 * messenger for validation layers.
		 *
		 * @param platform A reference to an IPlatform instance that provides
		 * platform-specific implementations for Vulkan device management
		 * operations. The platform is used to create the Vulkan instance,
		 * select the physical device, and create the logical device.
		 *
		 * @note The constructor performs several Vulkan initialization steps,
		 * including:
		 * - Creating a Vulkan instance using the platform's createInstance
		 * method.
		 * - Selecting a physical device using the platform's pickPhysicalDevice
		 * method.
		 * - Creating a logical device using the platform's createLogicalDevice
		 * method.
		 * - Creating a graphics queue and command pool for rendering
		 * operations.
		 * - Determining the maximum usable sample count for MSAA.
		 * - Setting up the Vulkan debug messenger for validation layers.
		 */
		DeviceContext(const IPlatform &platform);

		~DeviceContext();

		/**
		 * @brief Retrieves the number of MSAA samples used by this device
		 * context.
		 *
		 * @return VkSampleCountFlagBits The MSAA sample count flag bits
		 * representing the number of samples per pixel for multisample
		 * anti-aliasing operations.
		 */
		VkSampleCountFlagBits getMsaaSamples() const;

		/**
		 * @brief Retrieves a shared pointer to the ADeviceBackend instance
		 * associated with this device context.
		 *
		 * @return std::shared_ptr<ADeviceBackend> A shared pointer to the
		 * ADeviceBackend instance, which provides platform-specific
		 * implementations for Vulkan device management operations. This allows
		 * access to methods for creating Vulkan instances, logical devices, and
		 * picking physical devices, enabling abstraction and flexibility in the
		 * device context implementation.
		 */
		std::shared_ptr<ADeviceBackend> getDeviceBackend() const;

		/**
		 * @brief Retrieves the Vulkan command pool associated with this device
		 * context.
		 *
		 * @return VkCommandPool The Vulkan command pool used to allocate
		 * command buffers for recording rendering commands. The command pool is
		 * created during the initialization of the device context and is
		 * configured to allow individual command buffers to be reset.
		 */
		VkCommandPool getCommandPool() const;

		/**
		 * @brief Retrieves the Vulkan graphics queue associated with this
		 * device context.
		 *
		 * @return VkQueue The Vulkan graphics queue used to submit rendering
		 * commands to the GPU. The graphics queue is created during the
		 * initialization of the device context and is obtained from the logical
		 * device based on the determined graphics queue family index.
		 */
		VkQueue getGraphicsQueue() const;

		protected:
		/**
		 * The graphics queue, which is used to
		 * submit rendering commands to the GPU.
		 */
		VkQueue _graphicsQueue;

		/**
		 * The command pool, which is used to
		 * allocate command buffers for
		 * recording rendering commands.
		 */
		VkCommandPool _commandPool;

		/**
		 * The number of samples used for multisampling
		 * anti-aliasing (MSAA), which is a technique used
		 * to improve the visual quality of rendered
		 * images by reducing aliasing artifacts.
		 */
		VkSampleCountFlagBits _msaaSamples;

		/**
		 * A shared pointer to the ADeviceBackend instance,
		 * which provides platform-specific implementations
		 * for Vulkan device management operations.
		 * This allows the DeviceContext to utilize the backend's methods
		 */
		std::shared_ptr<ADeviceBackend> _deviceBackend;

		/*
		 * Vulkan debug messenger for validation layers.
		 * This member variable holds the handle to the Vulkan debug messenger,
		 * which is used to receive debug messages from the Vulkan validation
		 * layers. The debug messenger is set up during the initialization of
		 * the device context and is configured to capture messages of various
		 * severity levels and types, providing valuable information for
		 * debugging and development purposes.
		 *
		 * @note The debug messenger is created using the
		 * `vkCreateDebugUtilsMessengerEXT` function and requires the
		 * `VK_EXT_debug_utils` extension to be enabled in the Vulkan instance.
		 * Ensure that the extension is available and enabled for proper
		 * functionality of the debug messenger.
		 */
		VkDebugUtilsMessengerEXT _debugMessenger;

		private:
		/**
		 * @brief Determines and sets the maximum usable sample count for
		 * multi-sample anti-aliasing (MSAA).
		 *
		 * Queries the physical device properties to find the maximum sample
		 * count supported by both the framebuffer color and depth attachments.
		 * Sets _msaaSamples to the highest supported sample count, with a
		 * fallback to VK_SAMPLE_COUNT_1_BIT if no higher counts are available.
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
		 * @note This function should be called after the physical device has
		 * been selected.
		 * @note The _msaaSamples member variable is updated with the determined
		 * sample count.
		 */
		void getMaxUsableSampleCount();

		/**
		 * @brief Creates a Vulkan command pool for the device context.
		 *
		 * Initializes a command pool with the specified queue family index and
		 * flags. The command pool is configured to allow individual command
		 * buffers to be reset.
		 *
		 * @note The queue family index is currently hardcoded to 0, assuming
		 * the graphics queue family is at that index. This should be replaced
		 * with dynamic queue family selection in production code.
		 *
		 * @see vkCreateCommandPool
		 * @see VkCommandPoolCreateInfo
		 */
		void createCommandPool();

		/**
		 * @brief Creates the graphics queue for the device context.
		 *
		 * This function retrieves the graphics queue from the logical
		 * device and stores it in the _graphicsQueue member variable.
		 * It assumes that the logical device has already been created and
		 * that the graphics queue family index has been determined.
		 */
		void createGraphicsQueue();

		/**
		 * @brief Sets up the Vulkan debug messenger for the device context.
		 *
		 * This function initializes the Vulkan debug messenger, which is used
		 * to receive debug messages from the Vulkan validation layers. It
		 * configures the messenger to capture messages of various severity
		 * levels and types, and registers a callback function to handle the
		 * incoming messages.
		 */
		void setupDebugMessenger();

		/**
		 * @brief Populates a VkDebugUtilsMessengerCreateInfoEXT structure with
		 * the necessary information for creating a Vulkan debug messenger.
		 *
		 * @param createInfo Reference to a VkDebugUtilsMessengerCreateInfoEXT
		 * structure that will be populated with the debug messenger creation
		 * details.
		 * @param debugCallback A function pointer to the debug callback
		 * function that will handle debug messages from the Vulkan validation
		 * layers.
		 *
		 * The populated structure includes:
		 * - Message severity levels: Verbose, Warning, and Error.
		 * - Message types: General, Validation, and Performance.
		 * - The user-defined callback function for handling debug messages.
		 */
		void populateDebugMessengerCreateInfo(
			VkDebugUtilsMessengerCreateInfoEXT &createInfo,
			PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);

		/**
		 * @brief Creates a debug messenger for Vulkan instance debugging.
		 *
		 * This function wraps the Vulkan function
		 * `vkCreateDebugUtilsMessengerEXT` to create a debug messenger for
		 * capturing debug messages from the Vulkan validation layers.
		 *
		 * @param instance The Vulkan instance to associate the debug messenger
		 * with.
		 * @param pCreateInfo A pointer to a
		 * `VkDebugUtilsMessengerCreateInfoEXT` structure specifying the details
		 * of the debug messenger to be created.
		 * @param pAllocator A pointer to a `VkAllocationCallbacks` structure
		 * for custom memory allocation callbacks, or `nullptr` to use the
		 * default allocator.
		 * @param pDebugMessenger A pointer to a `VkDebugUtilsMessengerEXT`
		 * handle where the created debug messenger will be stored.
		 * @return `VK_SUCCESS` if the debug messenger was successfully created,
		 * or `VK_ERROR_EXTENSION_NOT_PRESENT` if the
		 * `vkCreateDebugUtilsMessengerEXT` function is not available.
		 *
		 * @note This function requires the `VK_EXT_debug_utils` extension to be
		 * enabled. Ensure that the extension is available and enabled in the
		 * Vulkan instance.
		 */
		VkResult createDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
			const VkAllocationCallbacks *pAllocator,
			VkDebugUtilsMessengerEXT *pDebugMessenger);
	};
}	 // namespace evan
