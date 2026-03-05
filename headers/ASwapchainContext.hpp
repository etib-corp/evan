/*
** ETIB PROJECT, 2026
** evan
** File description:
** ASwapchainContext
*/

#pragma once

#include "EvanPlatform.hpp"
#include "Version.hpp"

namespace evan
{
	/**
	 * @brief ASwapchainContext is an abstract class that represents a swapchain
	 * context in the Evan engine. It provides an interface for managing the
	 * swapchain, which is responsible for presenting rendered images to the
	 * screen. The ASwapchainContext class is designed to be inherited by
	 * specific implementations for different graphics APIs (e.g., DirectX,
	 * Vulkan, OpenGL). It defines the necessary functions and properties that
	 * must be implemented by derived classes to ensure proper functionality of
	 * the swapchain context.
	 *
	 * @note The ASwapchainContext class is an abstract base class and cannot be
	 * instantiated directly. It serves as a blueprint for creating specific
	 * swapchain context implementations that are compatible with the Evan
	 * engine's rendering system.
	 */
	class ASwapchainContext
	{
		public:
		    virtual ~ASwapchainContext();

		protected:
            VkRenderPass _renderPass;

	};
}	 // namespace evan
