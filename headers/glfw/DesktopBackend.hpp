/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopBackend
*/

#pragma once

#include "EvanPlatform.hpp"

#include "IDeviceBackend.hpp"
#include "DesktopPlatform.hpp"

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
	 * @brief Desktop backend implementation.
	 *
	 * This class provides an implementation of the backend for Desktop. It
	 * handles platform-specific functionality related to Desktop, such as
	 * creating a Vulkan surface for rendering or later handling input events.
	 */
	class DesktopBackend: public IDeviceBackend
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
		 * @brief Retrieves the Vulkan surface associated with the Desktop
		 * backend.
		 */
		VkSurfaceKHR _surface;

		VkInstance createInstance(const IPlatform &platform,
								  const std::string &appName,
								  Version &appVersion) override;

		VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice) override;

		VkPhysicalDevice pickPhysicalDevice(VkInstance instance) override;

		private:
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
