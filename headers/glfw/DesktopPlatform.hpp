/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrPlatform
*/

#pragma once

#include "IPlatform.hpp"

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
	class DesktopPlatform: public IPlatform
	{
		public:
		/**
		 * @brief Constructor for DesktopPlatform.
		 *
		 * @param name The name of the window.
		 * @param width The width of the window.
		 * @param height The height of the window.
		 */
		DesktopPlatform(const std::string &name, const uint32_t width,
						const uint32_t height);

		/**
		 * @brief Destructor for DesktopPlatform.
		 *
		 * Ensures proper cleanup of Desktop resources.
		 */
		~DesktopPlatform() override;

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
		std::vector<std::string> getRequiredInstanceExtensions() const override;

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

		GLFWwindow *_window = nullptr;	  // Pointer to the GLFW window
	};
}	 // namespace evan