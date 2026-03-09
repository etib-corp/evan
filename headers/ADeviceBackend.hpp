/*
** EPITECH PROJECT, 2026
** evan
** File description:
** ADeviceBackend
*/

#pragma once

#include "EvanPlatform.hpp"
#include "Version.hpp"

#include "IPlatform.hpp"
#include "QueueFamilyIndices.hpp"

#include <set>

namespace evan
{

	/**
	 * @brief Struct to encapsulate details about swap chain support.
	 *
	 * This struct contains information about the swap chain capabilities,
	 * available surface formats, and present modes for a Vulkan surface.
	 * It is typically used to query and store the swap chain support
	 * details for a Vulkan physical device.
	 *
	 * @struct SwapChainSupportDetails
	 */
	struct SwapChainSupportDetails {
		/*
		 * @brief Vulkan surface capabilities, such as the minimum and
		 * maximum image count, extent, and supported transforms.
		 */
		VkSurfaceCapabilitiesKHR capabilities;

		/*
		 * @brief A list of supported surface formats (color space and pixel
		 * format).
		 */
		std::vector<VkSurfaceFormatKHR> formats;

		/*
		 * @brief A list of supported presentation modes (e.g., FIFO,
		 * Mailbox, etc.).
		 */
		std::vector<VkPresentModeKHR> presentModes;
	};

	class ADeviceBackend
	{
		public:
		virtual ~ADeviceBackend();

		virtual void createInstance(const IPlatform &platform,
									const std::string &appName,
									Version &appVersion) = 0;

		virtual uint32_t countSwapchainFormats() = 0;
		virtual std::vector<int64_t> enumerateSwapchainFormats(uint32_t swapchainFormatCount) = 0;

		virtual void createLogicalDevice() = 0;

		virtual void pickPhysicalDevice() = 0;

		/**
		 * @brief Finds the queue family indices for the Vulkan physical device.
		 *
		 * This function queries the Vulkan API to find the indices of the queue
		 * families that support graphics and presentation operations. It
		 * returns a QueueFamilyIndices structure containing the indices of the
		 * graphics and presentation queue families, if they are found. If the
		 * required queue families are not found, the corresponding optional
		 * values in the QueueFamilyIndices structure will be empty.
		 */
		virtual evan::QueueFamilyIndices findQueueFamilies() = 0;

		/**
		 * @brief Retrieves the available Vulkan layers on the system.
		 *
		 * This function queries the Vulkan API to enumerate the available
		 * instance layers and returns a vector of VkLayerProperties structures
		 * containing information about each layer.
		 */
		std::vector<VkLayerProperties> getAvailableLayers();

		/**
		 * @brief Checks if the specified Vulkan physical device is suitable for
		 * use.
		 *
		 * This function evaluates the capabilities and features of the given
		 * Vulkan physical device to determine if it meets the requirements for
		 * the application. It checks for support for necessary queue families,
		 * device extensions, and swap chain support.
		 *
		 * @param device The Vulkan physical device to evaluate.
		 * @param surface The Vulkan surface to check for compatibility with the
		 * device.
		 * @param deviceExtensions A list of required device extensions that
		 * must be supported by the device.
		 * @return true if the device is suitable for use, false otherwise.
		 */
		bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface,
							  std::vector<const char *> deviceExtensions);

		/**
		 * @brief Queries the swap chain support details for a given Vulkan
		 * physical device and surface.
		 * This function retrieves information about the swap chain
		 * capabilities, available surface formats, and present modes for the
		 * specified Vulkan physical device and surface. It populates a
		 * SwapChainSupportDetails structure with the retrieved information.
		 * @param device The Vulkan physical device to query for swap chain
		 * support.
		 * @param surface The Vulkan surface to check for compatibility with the
		 * device.
		 * @return A SwapChainSupportDetails structure containing the swap chain
		 * support details for the specified device and surface.
		 */
		evan::SwapChainSupportDetails
			querySwapChainSupport(VkPhysicalDevice device,
														VkSurfaceKHR surface);

		VkInstance
			_VkInstance;	/// The Vulkan instance, which is the connection
							/// between the application and the Vulkan library.
							/// It is used to create and manage Vulkan resources
							/// and to query the capabilities of the Vulkan
							/// implementation.
		VkPhysicalDevice
			_physicalDevice;	/// The physical device, which represents a
								/// Vulkan-compatible GPU. It is used to query
								/// the capabilities of the GPU and to create
								/// logical devices for rendering operations.
		VkDevice _device;		/// The logical device, which represents an
								/// instance of a physical device and is used to
								/// perform rendering operations.
		private:
		/**
		 * @brief Checks if the specified Vulkan physical device supports the
		 * required device extensions. This function queries the Vulkan API to
		 * determine if the given physical device supports all of the required
		 * device extensions specified in the deviceExtensions vector.
		 * @param device The Vulkan physical device to check for extension
		 * support.
		 * @param deviceExtensions A list of required device extensions that
		 * must be supported by the device.
		 * @return true if the device supports all required extensions, false
		 * otherwise.
		 */
		bool checkDeviceExtensionSupport(
			VkPhysicalDevice device,
			std::vector<const char *> deviceExtensions);

	};
}	 // namespace evan