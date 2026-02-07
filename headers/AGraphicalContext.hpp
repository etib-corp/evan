/*
** ETIB PROJECT, 2025
** evan
** File description:
** AGraphicalContext
*/

#pragma once

#include "ARenderingContext.hpp"
#include "ASwapchainContext.hpp"

#include <memory>
#include <string>

namespace evan
{
	/**
	 * @brief Abstract base class representing a graphical context for Vulkan
	 * rendering.
	 *
	 * This class serves as a base for different graphical context
	 * implementations, such as those for different windowing systems (e.g.,
	 * GLFW, SDL). It provides an interface for initializing and managing the
	 * Vulkan instance, rendering context, and swapchain context.
	 *
	 * Derived classes must implement methods for creating the Vulkan instance
	 * and retrieving required instance extensions.
	 */
	class AGraphicalContext
	{
		public:
		virtual ~AGraphicalContext() = default;

		/**
		 * @brief Retrieves the list of required instance extensions for the
		 * graphical context.
		 *
		 * This pure virtual function must be implemented by derived classes to
		 * provide a list of Vulkan instance extensions that are necessary for
		 * the graphical context to function correctly. These extensions may
		 * include platform-specific surface extensions or other features
		 * required by the application.
		 *
		 * @return A vector of strings representing the names of the required
		 * instance extensions.
		 */
		virtual std::vector<std::string> getInstanceExtensions() = 0;

		/**
		 * @brief Retrieves the Vulkan context associated with the rendering
		 * context.
		 *
		 * This function provides access to the VulkanContext object, which is
		 * encapsulated within the rendering context. The VulkanContext is used
		 * for managing Vulkan-specific resources and operations.
		 *
		 * @return A constant reference to a shared pointer of VulkanContext.
		 */
		const std::shared_ptr<VulkanContext> &getVulkanContext() const
		{
			return _renderingContext->getVulkanContext();
		}

		/**
		 * @brief Retrieves the Vulkan instance associated with this graphical
		 * context.
		 *
		 * @return A constant reference to the Vulkan instance (VkInstance).
		 */
		const VkInstance &getInstance() const
		{
			return _instance;
		}

		/**
		 * @brief Sets the swapchain context for the graphical context.
		 *
		 * @param swapchainContext A shared pointer to an ASwapchainContext
		 * instance that represents the swapchain context to be set.
		 */
		virtual void setSwapchainContext(
			const std::shared_ptr<ASwapchainContext> &swapchainContext)
		{
			_swapchainContext = swapchainContext;
		}

		protected:
		/**
		 * @brief Creates and initializes the Vulkan instance for the graphical
		 * context.
		 *
		 * This pure virtual function must be implemented by derived classes to
		 * set up the Vulkan instance, including specifying application
		 * information, enabled extensions, and validation layers as needed.
		 *
		 * @note This function should be called before any Vulkan operations are
		 * performed.
		 */
		virtual void createInstance() = 0;

		std::shared_ptr<ARenderingContext>
			_renderingContext;	  // Rendering context managing Vulkan resources
		std::shared_ptr<ASwapchainContext>
			_swapchainContext;	  // Swapchain context for managing
								  // swapchain-related resources
		VkInstance _instance =
			VK_NULL_HANDLE;	   // Vulkan instance for the graphical context
	};
}	 // namespace evan
