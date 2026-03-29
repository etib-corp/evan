/*
** EPITECH PROJECT, 2026
** evan
** File description:
** IDesktopPlatform
*/

#pragma once

#include "IPlatform.hpp"
#include "glfw/DesktopBackend.hpp"
#include "glfw/DesktopSwapchainContext.hpp"

#include <iostream>

namespace evan
{
	/**
	 * @brief Desktop platform implementation.
	 *
	 * This class provides an implementation of the IPlatform interface for
	 * Desktop. It handles platform-specific functionality related to Desktop
	 * using GLFW for window management and event handling.
	 */
	class IDesktopPlatform: public IPlatform
	{
		public:

		/**
		 * @brief Destructor for DesktopPlatform.
		 *
		 * Ensures proper cleanup of Desktop resources.
		 */
		~IDesktopPlatform() override;

		/**
		 * @brief Get the required instance extensions for the Desktop platform.
		 *
		 * @return A vector of strings representing the required instance
		 * extensions for the Desktop platform.
		 *
		 * This method retrieves the necessary Vulkan instance extensions
		 * required for the Desktop platform, which may include extensions for
		 * window surface creation and other platform-specific features.
		 */
		virtual std::vector<std::string> getRequiredInstanceExtensions() const = 0;

		/**
		 * @brief Check if the Desktop platform should close.
		 *
		 * @return true if the Desktop platform should close, false otherwise.
		 */
		bool shouldClose() const override;

		/**
		 * @brief Poll events for the Desktop platform.
		 *
		 * @param deviceBackend The device backend to use for polling events.
		 *
		 * This method processes events for the Desktop platform, such as user
		 * input and window events, using the provided device backend. It
		 * ensures that the application remains responsive to user interactions
		 * and system events.
		 *
		 * Note: The implementation of this method may involve using GLFW's
		 * event polling functions to handle events specific to the Desktop
		 * platform.
		 */
		void pollEvents(ADeviceBackend &deviceBackend) override;

		/**
		 * @brief Create a Vulkan surface for the Desktop platform.
		 *
		 * @param instance The Vulkan instance to use for creating the surface.
		 * @return The created Vulkan surface handle.
		 *
		 * This method creates a Vulkan surface that is compatible with the
		 * Desktop platform, allowing rendered images to be presented on the
		 * screen. The implementation may involve using GLFW's functions to
		 * create a window surface that can be used with Vulkan.
		 */
		virtual VkSurfaceKHR createSurface(VkInstance instance) const = 0;

		/**
		 * @brief Create a device context for the Desktop platform.
		 *
		 * @return A shared pointer to the created device context for the
		 * Desktop platform.
		 *
		 * This method creates a device context specific to the Desktop platform,
		 * which is responsible for managing Vulkan device resources and
		 * operations. The implementation may involve initializing Vulkan and
		 * setting up the necessary resources for rendering on the Desktop
		 * environment.
		 */
		std::shared_ptr<ADeviceBackend> createDeviceBackend() const override;

		/**
		 * @brief Create a swapchain context for the Desktop platform.
		 *
		 * @param deviceContext The device context to use for creating the
		 * swapchain context.
		 * @return A shared pointer to the created swapchain context.
		 *
		 * This method creates a swapchain context specific to the Desktop
		 * platform, which is responsible for managing the presentation of
		 * rendered images to the screen. The implementation may involve
		 * creating a Vulkan swapchain and associated resources tailored to the
		 * Desktop environment.
		 */
		std::shared_ptr<ASwapchainContext> createSwapchainContext(
			const DeviceContext &deviceContext) const override;

		GLFWwindow *_window = nullptr;	  // Pointer to the GLFW window
	};
}	 // namespace evan