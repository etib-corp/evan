/*
** EPITECH PROJECT, 2026
** evan
** File description:
** IXrPlatform
*/

#pragma once

#include "IPlatform.hpp"
#include "DeviceContext.hpp"
#include "openxr/XrDeviceBackend.hpp"
#include "openxr/XrSwapchainContext.hpp"

#include <iostream>

namespace evan
{
	/**
	 * @brief OpenXR platform implementation.
	 *
	 * This class provides an implementation of the IPlatform interface for
	 * OpenXR. It handles platform-specific functionality related to OpenXR.
	 */
	class IXrPlatform: public IPlatform
	{
		public:

		/**
		 * @brief Virtual destructor.
		 *
		 * Ensures proper cleanup of derived class resources.
		 */
		virtual ~IXrPlatform() = default;

		/**
		 * @brief Get the required instance extensions for the OpenXR platform.
		 *
		 * This function returns a vector of strings representing the required
		 * instance extensions for the OpenXR platform. These extensions are
		 * necessary for proper functionality and compatibility with the OpenXR
		 * runtime on the target platform.
		 *
		 * @return A vector of strings containing the required instance
		 * extensions for the OpenXR platform.
		 */
		virtual std::vector<std::string> getRequiredInstanceExtensions() const = 0;

		/**
		 * @brief Check if the OpenXR platform should close.
		 *
		 * This function checks the internal state of the XrPlatform to
		 * determine if it should close. This is typically based on events such
		 * as session state changes or user input that indicates the application
		 * should exit.
		 *
		 * @return true if the platform should close, false otherwise.
		 */
		bool shouldClose() const override;

		/**
		 * @brief Poll for OpenXR events and handle them accordingly.
		 *
		 * This function polls for events from the OpenXR runtime and processes
		 * them. It handles events such as session state changes, input events,
		 * and other relevant OpenXR events. The function may update internal
		 * state based on the events received, such as setting the shouldClose
		 * flag when a session state change indicates that the application
		 * should exit.
		 *
		 * @param deviceBackend A reference to the ADeviceBackend, which may be
		 * used for handling device-specific operations related to the events
		 * being processed.
		 *
		 * @note This function should be called regularly in the main
		 * application loop to ensure that events are processed in a timely
		 * manner.
		 */
		void pollEvents(ADeviceBackend &deviceBackend) override;

		/**
		 * @brief Create a device context for the OpenXR platform.
		 *
		 * This function creates and returns a shared pointer to a DeviceContext
		 * that is compatible with the OpenXR platform. The created device context
		 * should be initialized with the necessary resources and configurations
		 * required for rendering and interacting with the OpenXR runtime.
		 *
		 * @return A shared pointer to the created DeviceContext for the OpenXR
		 * platform.
		 */
		std::shared_ptr<ADeviceBackend> createDeviceBackend() const override;

		/**
		 * @brief Create a swapchain context for the OpenXR platform.
		 *
		 * This function creates and returns a shared pointer to an ASwapchainContext
		 * that is compatible with the OpenXR platform. The created swapchain context
		 * should be initialized with the necessary resources and configurations
		 * required for rendering to the OpenXR runtime's swapchain.
		 *
		 * @param deviceContext A reference to the DeviceContext that may be used
		 * for initializing the swapchain context with the appropriate device
		 * resources.
		 *
		 * @return A shared pointer to the created ASwapchainContext for the
		 * OpenXR platform.
		 */
		std::shared_ptr<ASwapchainContext> createSwapchainContext(
			const DeviceContext &deviceContext) const override;

		/**
		 * @brief Retrieves the Android-specific instance creation
		 * information.
		 *
		 * This function returns a pointer to the Android-specific instance
		 * creation structure, cast to a generic XrBaseInStructure pointer.
		 * This is typically used when creating an OpenXR instance on an
		 * Android platform.
		 *
		 * @return A pointer to the Android-specific instance creation
		 * information as an XrBaseInStructure.
		 */
		virtual const XrBaseInStructure *getInstanceCreateInfo() const = 0;

		protected:
		/**
		 * @brief Process an OpenXR session state change event.
		 *
		 * This function handles the XrEventDataSessionStateChanged event,
		 * updating the internal state of the XrPlatform based on the new
		 * session state. It may set flags such as shouldClose or sessionRunning
		 * based on the session state received in the event data.
		 *
		 * @param eventData The event data containing information about the
		 * session state change.
		 * @param session The OpenXR session associated with the event.
		 *
		 * @note This function is typically called from within the pollEvents
		 * method when a session state change event is received. It is
		 * responsible for ensuring that the XrPlatform responds appropriately
		 * to changes in the OpenXR session state.
		 */
		void processSessionStateChangedEvent(
			const XrEventDataSessionStateChanged &eventData, XrSession session);

		/**
		 * Flag to indicate if the platform should close.
		 *
		 * OpenXR specific.
		 */
		bool _shouldClose = false;

		/**
		 * Flag to track if the session is currently running.
		 *
		 * This is used to determine if the application should continue running
		 * or if it should exit based on the session state changes received from
		 * the OpenXR runtime.
		 */
		bool _sessionRunning = false;
	};
}	 // namespace evan