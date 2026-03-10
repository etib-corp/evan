/*
** EPITECH PROJECT, 2026
** evan
** File description:
** ASwapchainContext
*/

#pragma once

#include "EvanPlatform.hpp"
#include "ADeviceBackend.hpp"

#include <map>
#include <memory>
#include <iostream>

namespace evan
{

	/**
	 * @class ASwapchainContext
	 * @brief Represents the context for managing the swapchain in the rendering
	 * system.
	 *
	 * The ASwapchainContext class encapsulates the functionality related to
	 * managing the swapchain in the rendering system. It is responsible for
	 * handling the creation, configuration, and management of the swapchain,
	 * which is a series of images used for presenting rendered frames to the
	 * display. This class serves as a central point for managing
	 * swapchain-related operations in the engine.
	 *
	 */
	class ASwapchainContext
	{
		public:
		virtual ~ASwapchainContext() = default;

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

		protected:
		VkRenderPass _renderPass;

		private:
		void createRenderPass(const ADeviceBackend &deviceBackend,
							  VkSampleCountFlagBits msaaSamples);
		VkFormat
			selectSwapchainFormat(const std::vector<int64_t> &swapchainFormats);
		VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
		VkFormat findSupportedFormat(
			VkPhysicalDevice physicalDevice,
			const std::vector<VkFormat> &candidates, VkImageTiling tiling,
			VkFormatFeatureFlags features);
	};
}	 // namespace evan