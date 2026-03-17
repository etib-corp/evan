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

        /**
         * @struct CreateXrSwapchainImageProperties
         * @brief Encapsulates properties for creating an XrSwapchainImage instance.
         * This structure holds all necessary parameters for initializing an XrSwapchainImage, including the OpenXR swapchain handle, swapchain creation info, Vulkan render pass, and a reference to the device context. It serves as a convenient way to pass multiple parameters to the XrSwapchainImage constructor while ensuring that all required information is provided for proper initialization of the swapchain image.
         */
        struct CreateXrSwapchainImageProperties {
            /*
            * @brief The OpenXR swapchain handle associated with the swapchain images.
            */
            XrSwapchain swapchain;
            /*
            * @brief The swapchain creation info structure containing parameters for creating the swapchain images, such as format, dimensions, sample count, and usage flags.
            */
            XrSwapchainCreateInfo createInfo;
            /*
            * @brief The Vulkan render pass used for rendering operations with the swapchain images. It defines the rendering pipeline and how the images will be used during rendering.
            */
            VkRenderPass renderPass;
            /*
            * @brief A reference to the device context, which provides access to Vulkan device resources and
operations needed for managing the swapchain images, such as creating image views, allocating memory, and performing layout transitions.
            */
            const DeviceContext &deviceContext;
        };

        XrSwapchainImage(const CreateXrSwapchainImageProperties &properties);

        void destroy(VkDevice device) override;

        XrSwapchain _swapchain; // The OpenXR swapchain handle associated with the swapchain images
    private:
        std::vector<XrSwapchainImageVulkan2KHR>
				_swapchainImages;
        XrSwapchainImageBaseHeader *_swapchainImagesBase;	// Base pointer for swapchain images
    };
} // namespace evan