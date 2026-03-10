/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrSwapchainImage
*/

#pragma once

#include "ASwapchainImage.hpp"
#include "DeviceContext.hpp"

namespace evan
{
    class XrSwapchainImage : public ASwapchainImage
    {
    public:
        XrSwapchainImage();
    private:
        VkViewport _viewport = {
            0, 0, 0, 0, 0, 1.0
        };	  // Viewport for rendering
        VkRect2D _scissor = {
            { 0, 0 }, { 0, 0 }
        };	  // Scissor rectangle for rendering
        std::vector<XrSwapchainImageVulkan2KHR>
				_swapchainImages;
    };
} // namespace evan