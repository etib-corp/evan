/*
** EPITECH PROJECT, 2026
** evan
** File description:
** IDeviceBackend
*/

#pragma once

#include "EvanPlatform.hpp"
#include "Version.hpp"

#include "IPlatform.hpp"

#include <optional>
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

	/**
	 * @brief A structure to represent Vulkan queue family indices.
	 *
	 * This structure is used to store the indices of queue families
	 * that support specific operations, such as graphics and presentation.
	 *
	 */
	struct QueueFamilyIndices {
		/*
		 * An optional value representing the index of the queue family
		 * that supports graphics operations.
		 */
		std::optional<uint32_t> graphicsFamily;

		/*
		 * An optional value representing the index of the queue family
		 * that supports presentation operations.
		 */
		std::optional<uint32_t> presentFamily;

		/*
		 * @brief isComplete
		 *
		 * A method to check if both graphicsFamily and presentFamily
		 * have been assigned values.
		 *
		 * @return true if both graphicsFamily and presentFamily have
		 * values, indicating that the required queue families are
		 * available. False otherwise.
		 */
		bool isComplete() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	class IDeviceBackend
	{
		public:
		virtual ~IDeviceBackend() = default;

		virtual void init(const IPlatform &platform) = 0;

		virtual VkInstance createInstance(const IPlatform &platform,
										  const std::string &appName,
										  Version &appVersion) = 0;

		virtual VkDevice
			createLogicalDevice(VkPhysicalDevice physicalDevice) = 0;

		virtual VkPhysicalDevice pickPhysicalDevice(VkInstance instance) = 0;

		/**
		 * @brief Finds the queue families supported by a given Vulkan physical
		 * device.
		 *
		 * This function iterates through the queue families of the specified
		 * physical device and identifies the indices of queue families that
		 * support specific capabilities, such as graphics operations.
		 *
		 * @param device The Vulkan physical device to query for queue family
		 * properties.
		 * @return A QueueFamilyIndices structure containing the indices of the
		 * queue families that meet the required criteria. If no suitable queue
		 * families are found, the indices will remain unset.
		 */
		evan::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

		/**
		 * @brief Finds the queue families that support specific operations on a
		 * given Vulkan physical device and surface.
		 *
		 * This function identifies the queue families that support graphics
		 * operations and presentation to a given surface. It iterates through
		 * the queue families of the specified physical device and checks their
		 * capabilities.
		 *
		 * @param device The Vulkan physical device to query for queue family
		 * properties.
		 * @param surface The Vulkan surface to check for presentation support.
		 * @return QueueFamilyIndices A structure containing the indices of the
		 * graphics and presentation queue families. If no suitable queue
		 * families are found, the indices will remain incomplete.
		 */
		evan::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,
												   VkSurfaceKHR surface);

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
		bool evan::IDeviceBackend::checkDeviceExtensionSupport(
			VkPhysicalDevice device,
			std::vector<const char *> deviceExtensions);

		/**
		 * @brief Queries the swap chain support details for a given Vulkan
		 * physical device and surface.
		 * This function retrieves information about the swap chain capabilities,
		 * available surface formats, and present modes for the specified Vulkan
		 * physical device and surface. It populates a SwapChainSupportDetails
		 * structure with the retrieved information.
		 * @param device The Vulkan physical device to query for swap chain
		 * support.
		 * @param surface The Vulkan surface to check for compatibility with the
		 * device.
		 * @return A SwapChainSupportDetails structure containing the swap chain
		 * support details for the specified device and surface.
		 */
		evan::SwapChainSupportDetails
			evan::IDeviceBackend::querySwapChainSupport(VkPhysicalDevice device,
														VkSurfaceKHR surface);
	};
}	 // namespace evan