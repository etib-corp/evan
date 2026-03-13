/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopBackend
*/

#pragma once

#include "EvanPlatform.hpp"

#include "ADeviceBackend.hpp"
#include "DesktopPlatform.hpp"

#include "Version.hpp"

/**
 * @brief List of Vulkan device extensions to enable.
 *
 * This vector contains the names of device extensions that will be
 * enabled for the Vulkan logical device. Device extensions provide
 * additional functionality beyond the core Vulkan specification. The
 * "VK_KHR_swapchain" extension is essential for presenting rendered
 * images to a display surface, while "VK_KHR_portability_subset" is
 * used to ensure compatibility across different platforms, particularly
 * on macOS where Vulkan support is provided through MoltenVK.
 */
const std::vector<const char *> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset"
};

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
	 * @brief Desktop backend implementation.
	 *
	 * This class provides an implementation of the backend for Desktop. It
	 * handles platform-specific functionality related to Desktop, such as
	 * creating a Vulkan surface for rendering or later handling input events.
	 */
	class DesktopBackend: public ADeviceBackend
	{
		public:
		/**
		 * @brief Constructor for DesktopBackend.
		 *
		 * @param platform The platform instance to be used for Desktop
		 * initialization.
		 */
		DesktopBackend(const DesktopPlatform &platform);

		/**
		 * @brief Destructor for DesktopBackend.
		 *
		 * Ensures proper cleanup of Desktop resources.
		 */
		~DesktopBackend();

		/**
		 * @brief Creates a Vulkan surface for rendering.
		 *
		 * @param instance The Vulkan instance to be used for surface creation.
		 * @param window The GLFW window for which the surface will be created.
		 * @return The created Vulkan surface.
		 */
		VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window);

		/**
		 * @brief Creates the presentation queue for the device context.
		 *
		 * This function retrieves the presentation queue from the logical
		 * device and stores it in a member variable (not shown in this
		 * snippet). It assumes that the logical device has already been created
		 * and that the presentation queue family index has been determined.
		 */
		void createPresentQueue();

		uint32_t countSwapchainFormats() const override;
		std::vector<int64_t> enumerateSwapchainFormats(
			uint32_t swapchainFormatCount) const override;

		/**
		 * @brief Retrieves the Vulkan surface associated with the Desktop
		 * backend.
		 */
		VkSurfaceKHR _surface;

		VkQueue _presentQueue;	  // Vulkan queue for presentation

		void createInstance(const IPlatform &platform,
							const std::string &appName,
							Version &appVersion) override;

		void createLogicalDevice() override;

		void pickPhysicalDevice() override;

		QueueFamilyIndices findQueueFamilies() override;

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

		private:
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

		/**
		 * @brief Checks if the required validation layers are supported.
		 *
		 * @return true if all requested validation layers are supported, false
		 * otherwise.
		 */
		bool checkValidationLayerSupport();

		/**
		 * @brief Retrieves the required instance extensions for the Desktop
		 * backend.
		 *
		 * @return A vector of strings containing the names of the required
		 * instance extensions.
		 */
		std::vector<std::string> getInstanceExtensions();

		/**
		 * @brief Populates the debug messenger create info structure.
		 *
		 * @param createInfo The structure to be populated with debug messenger
		 * information.
		 * @param debugCallback The callback function to be used for debug
		 * messages.
		 */
		void populateDebugMessengerCreateInfo(
			VkDebugUtilsMessengerCreateInfoEXT &createInfo,
			debugCallback_t debugCallback);
	};
}	 // namespace evan
