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
		VkSurfaceKHR surface;

		VkInstance createInstance(const IPlatform &platform,
								  const std::string &appName,
								  Version &appVersion) override;

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
