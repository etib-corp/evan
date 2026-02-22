/*
** EPITECH PROJECT, 2026
** evan
** File description:
** SwapchainContext
*/

#include "SwapchainContext.hpp"

evan::SwapchainImageContext::~SwapchainImageContext()
{
    // Cleanup code for depth and color resources, image views, and swapchain images
}

void evan::SwapchainImageContext::createDepthResources()
{
	// Utils::CreateImageProperties imageProperties = {
	// 	._logicalDevice	 = _logicalDevice,
	// 	._physicalDevice = _physicalDevice,
	// 	._width			 = _extent.width,
	// 	._height		 = _extent.height,
	// 	._mipLevels		 = 1,	 // No mipmaps for color attachment
	// 	._numSamples	 = _msaaSamples,
	// 	._format		 = _swapchainColorFormat,
	// 	._tiling		 = VK_IMAGE_TILING_OPTIMAL,
	// 	._usage			 = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
	// 		| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	// 	._properties  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	// 	._image		  = _colorImage,
	// 	._imageMemory = _colorImageMemory
	// };
}

evan::SwapchainContext::~SwapchainContext()
{
    // vkDestroyRenderPass(, _renderPass, nullptr);
}