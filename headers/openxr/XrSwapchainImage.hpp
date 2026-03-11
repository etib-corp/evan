/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrSwapchainImage
*/

#pragma once

#include "ASwapchainImage.hpp"
#include "DeviceContext.hpp"

#include <openxr/openxr.h>

namespace evan
{
    class XrSwapchainImage : public ASwapchainImage
    {
    public:
        XrSwapchainImage(const DeviceContext &deviceContext, XrSwapchain swapchain, XrSwapchainCreateInfo createInfo);
    private:
        VkViewport _viewport = {
            0, 0, 0, 0, 0, 1.0
        };	  // Viewport for rendering
        VkRect2D _scissor = {
            { 0, 0 }, { 0, 0 }
        };	  // Scissor rectangle for rendering
        std::vector<XrSwapchainImageVulkan2KHR>
				_swapchainImages;
        XrSwapchainImageBaseHeader *_swapchainImagesBase;	// Base pointer for swapchain images
    };
} // namespace evan