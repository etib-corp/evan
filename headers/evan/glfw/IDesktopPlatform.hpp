/*
** ETIB PROJECT, 2026
** evan
** File description:
** IDesktopPlatform
*/

#pragma once

#include "evan/IPlatform.hpp"
#include "evan/glfw/DesktopBackend.hpp"
#include "evan/glfw/DesktopSwapchainContext.hpp"

#include <utility/event/keyboard_event.hpp>
#include <utility/event/mouse_button_event.hpp>
#include <utility/event/mouse_motion_event.hpp>
#include <utility/event/mouse_wheel_event.hpp>
#include <utility/event/cursor_enter_event.hpp>
#include <utility/event/file_drop_event.hpp>
#include <utility/event/text_input_event.hpp>

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
		virtual std::vector<std::string>
			getRequiredInstanceExtensions() const override = 0;

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
		 *
		 * @return A vector of shared pointers to Event objects representing the
		 * events that were polled from the Desktop platform. Each Event object
		 * contains information about the type of event, such as input events.
		 */
		virtual std::vector<std::shared_ptr<utility::event::Event>>
			pollEvents(ADeviceBackend &deviceBackend) override;

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
		 * This method creates a device context specific to the Desktop
		 * platform, which is responsible for managing Vulkan device resources
		 * and operations. The implementation may involve initializing Vulkan
		 * and setting up the necessary resources for rendering on the Desktop
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

		/**
		 * @brief Convert a GLFW key code to a KeyboardEvent::KeyCode.
		 *
		 * @param glfwKey The GLFW key code to convert.
		 *
		 * @return The corresponding KeyboardEvent::KeyCode for the given GLFW
		 * key code.
		 */
		utility::event::KeyboardEvent::KeyCode
			convertGlfwKeyToKeyCode(int glfwKey) const;

		/**
		 * @brief Convert a GLFW mouse button code to a
		 * MouseButtonEvent::Button.
		 *
		 * @param glfwMouseButton The GLFW mouse button code to convert.
		 *
		 * @return The corresponding MouseButtonEvent::Button for the given GLFW
		 * mouse button code.
		 */
		utility::event::MouseButtonEvent::Button
			convertGlfwMouseButtonToButton(int glfwMouseButton) const;

		/**
		 * @brief Convert GLFW modifier flags to KeyModifiers.
		 *
		 * @param glfwMods The GLFW modifier flags to convert.
		 *
		 * @return The corresponding KeyModifiers for the given GLFW modifier
		 * flags.
		 */
		utility::event::KeyboardEvent::KeyModifiers
			convertGlfwModsToKeyModifiers(int glfwMods) const;

		/**
		 * @brief Get the current mouse position.
		 *
		 * @return The current mouse position as a MousePosition (2D vector).
		 *
		 * This method retrieves the current position of the mouse cursor on the
		 * screen, which can be used for handling mouse motion events and
		 * interactions within the application.
		 */
		utility::event::MouseMotionEvent::MousePosition
			getMousePosition() const;

		/**
		 * @brief Get the current state of movement keys.
		 *
		 * Polls the current state of movement keys (WASD, QE, arrows) and
		 * returns keyboard events for keys that are currently pressed.
		 * This enables continuous movement while holding keys.
		 *
		 * @return Vector of keyboard events for currently pressed movement
		 * keys.
		 */
		std::vector<std::shared_ptr<utility::event::KeyboardEvent>>
			getPressedMovementKeys() const;

		/**
		 * @brief Currently active GLFW window for the Desktop platform.
		 */
		GLFWwindow *_window = nullptr;

		/**
		 * @brief Vector to store keyboard events.
		 *
		 * This vector holds unique pointers to KeyboardEvent objects, which
		 * represent keyboard input events that have been polled from the GLFW
		 * event system. Each KeyboardEvent contains information about the key
		 * that was pressed or released, as well as any modifier keys that were
		 * active at the time of the event. This allows the application to
		 * process keyboard input in a structured way, responding to user
		 * interactions effectively.
		 */
		std::vector<std::shared_ptr<utility::event::KeyboardEvent>>
			_keyboardEvents;

		/**
		 * @brief Vector to store mouse button events.
		 *
		 * This vector holds unique pointers to MouseButtonEvent objects, which
		 * represent mouse button input events that have been polled from the
		 * GLFW event system. Each MouseButtonEvent contains information about
		 * which mouse button was pressed or released, as well as the current
		 * state of all mouse buttons and the position of the mouse cursor at
		 * the time of the event. This allows the application to process mouse
		 * button input in a structured way, responding to user interactions
		 * effectively.
		 */
		std::vector<std::shared_ptr<utility::event::MouseButtonEvent>>
			_mouseButtonEvents;

		/**
		 * @brief Vector to store mouse motion events.
		 *
		 * This vector holds unique pointers to MouseMotionEvent objects, which
		 * represent mouse motion events that have been polled from the GLFW
		 * event system. Each MouseMotionEvent contains information about the
		 * current position of the mouse cursor, allowing the application to
		 * respond to mouse movement and interactions effectively.
		 */
		std::vector<std::shared_ptr<utility::event::MouseMotionEvent>>
			_mouseMotionEvents;

		/**
		 * @brief Vector to store mouse wheel events.
		 *
		 * This vector holds unique pointers to MouseWheelEvent objects, which
		 * represent mouse wheel scroll events that have been polled from the
		 * GLFW event system. Each MouseWheelEvent contains information about
		 * the scroll offset, allowing the application to respond to mouse wheel
		 * scrolling interactions effectively.
		 */
		std::vector<std::shared_ptr<utility::event::MouseWheelEvent>>
			_mouseWheelEvents;

		/**
		 * @brief Vector to store cursor enter/leave events.
		 *
		 * This vector holds unique pointers to CursorEnterEvent objects, which
		 * represent cursor enter/leave events that have been polled from the
		 * GLFW event system. Each CursorEnterEvent contains information about
		 * whether the cursor entered or left the window's content area.
		 */
		std::vector<std::shared_ptr<utility::event::CursorEnterEvent>>
			_cursorEnterEvents;

		/**
		 * @brief Vector to store file drop events.
		 *
		 * This vector holds unique pointers to FileDropEvent objects, which
		 * represent file drop events that have been polled from the GLFW event
		 * system. Each FileDropEvent contains information about the paths of
		 * files and/or directories dropped on the window.
		 */
		std::vector<std::shared_ptr<utility::event::FileDropEvent>>
			_fileDropEvents;

		/**
		 * @brief Vector to store text input events.
		 *
		 * This vector holds unique pointers to TextInputEvent objects, which
		 * represent text input events that have been polled from the GLFW event
		 * system. Each TextInputEvent contains committed UTF-8 text input from
		 * the operating system's text input system.
		 */
		std::vector<std::shared_ptr<utility::event::TextInputEvent>>
			_textInputEvents;

		private:
	};
}	 // namespace evan