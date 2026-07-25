/*
** ETIB PROJECT, 2026
** evan
** File description:
** IPlatform
*/

#pragma once

#include "evan/EvanPlatform.hpp"

#include <utility/event/event.hpp>

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include <string>

namespace evan
{
	class ADeviceBackend;
	class DeviceContext;
	class ASwapchainContext;

	/**
	 * @brief Abstract interface for platform-specific functionality.
	 *
	 * Defines a common interface for platform implementations. Derived classes
	 * must implement platform-specific behavior.
	 */
	class IPlatform:
		protected utility::logging::Loggable<IPlatform,
											 utility::logging::DefaultLogger>
	{
		public:
		/**
		 * @brief Virtual destructor.
		 *
		 * Ensures proper cleanup of derived class resources.
		 */
		virtual ~IPlatform() = default;

		/**
		 * @brief Get the required instance extensions for the platform.
		 *
		 * @return A vector of strings representing the required instance
		 * extensions.
		 */
		virtual std::vector<std::string>
			getRequiredInstanceExtensions() const = 0;

		/**
		 * @brief Check if the platform should close.
		 *
		 * @return true if the platform should close, false otherwise.
		 */
		virtual bool shouldClose() const = 0;

		/**
		 * @brief Poll for platform events.
		 *
		 * This method should be called regularly to process platform events.
		 *
		 * @param deviceBackend The device backend to use for event processing,
		 * if needed.
		 * @return A vector of shared pointers to Event objects representing the
		 * events that were polled from the platform. Each Event object contains
		 * information about the type of event, such as input events.
		 */
		virtual std::vector<std::shared_ptr<utility::event::Event>>
			pollEvents(ADeviceBackend &deviceBackend) = 0;

		/**
		 * @brief Create a device context for the platform.
		 *
		 * @return A shared pointer to the created device context.
		 *
		 * This method should create and return a device context that is
		 * compatible with the platform's requirements and capabilities.
		 */
		virtual std::shared_ptr<ADeviceBackend> createDeviceBackend() const = 0;

		/**
		 * @brief Get platform-specific instance creation information.
		 *
		 * @param deviceContext The device context to use for creating
		 * the swapchain context.
		 *
		 * @return A pointer to an XrBaseInStructure containing instance
		 * creation information, or nullptr if not applicable.
		 *
		 * This method can be used to provide additional instance creation
		 * information specific to the platform, such as application info or
		 * extension-specific structures.
		 */
		virtual std::shared_ptr<ASwapchainContext> createSwapchainContext(
			const DeviceContext &deviceContext) const = 0;
	};
}	 // namespace evan