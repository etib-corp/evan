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
    /**
     * @class XrSwapchainImage
     *
     * @brief Represents a swapchain image for OpenXR rendering.
     *
     * This class encapsulates the functionality for managing swapchain images in the context of OpenXR rendering. It inherits from ASwapchainImage and provides implementations for destroying the swapchain images and filling presentation information. The class holds a reference to the OpenXR swapchain handle, the dimensions of the swapchain images, and a vector of Vulkan swapchain image structures. It also includes a structure for encapsulating properties required for creating an instance of XrSwapchainImage, which includes the OpenXR swapchain handle, swapchain creation info, Vulkan render pass, and a reference to the device context.
     *
     * @note The XrSwapchainImage class is designed to work within the rendering system of the project, providing a way to manage swapchain images for OpenXR applications. It is responsible for handling the lifecycle of the swapchain images, including their creation, destruction, and presentation to the screen.
     */
    class XrSwapchainImage : public ASwapchainImage
    {
    public:
        /**
         * @struct CreateXrSwapchainImageProperties
         *
         * @brief Encapsulates properties for creating an XrSwapchainImage instance.
         *
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
             * operations needed for managing the swapchain images, such as creating image views, allocating memory, and performing layout transitions.
            */
            const DeviceContext &deviceContext;
        };

        /**
         * @brief Constructs an XrSwapchainImage instance using the provided properties.
         *
         * This constructor initializes the XrSwapchainImage by creating Vulkan image views for each swapchain image based on the provided OpenXR swapchain handle and creation info. It also sets up the necessary resources for rendering operations, such as determining the dimensions of the swapchain images and storing references to the Vulkan render pass and device context for later use in rendering and presentation operations.
         *
         * @param properties A structure containing all necessary properties for initializing the XrSwapchainImage, including the OpenXR swapchain handle, swapchain creation info, Vulkan render pass, and a reference to the device context.
         *
         * @note The constructor performs several initialization steps, including:
         * - Retrieving the number of swapchain images from the OpenXR swapchain handle.
         * - Creating Vulkan image views for each swapchain image based on the provided creation info.
         * - Storing the dimensions of the swapchain images for later use in rendering and presentation operations.
         * - Setting up references to the Vulkan render pass and device context for use in rendering operations.
         * - Handling any necessary error checking and resource management to ensure that the swapchain images are properly initialized and ready for use in the rendering system.
         */
        XrSwapchainImage(const CreateXrSwapchainImageProperties &properties);

        /**
         * @brief Destroys the swapchain images and releases associated resources.
         *
         * This function overrides the pure virtual function from ASwapchainImage to handle the destruction of the swapchain images and the release of any resources associated with them. It takes a Vulkan device as a parameter, which is used to perform the necessary cleanup operations, such as destroying image views and freeing memory. Implement this function to ensure that all resources related to the swapchain images are properly released when they are no longer needed.
         *
         * @param device The Vulkan device used to perform the cleanup operations for the swapchain images.
         *
         * @note This function is responsible for ensuring that all Vulkan resources associated with the swapchain images are properly released to prevent memory leaks and ensure efficient resource management in the rendering system.
         */
        void destroy(VkDevice device) override;

        /**
         * @brief Fills the VkPresentInfoKHR structure with the necessary information for presenting the swapchain images.
         *
         * This function overrides the pure virtual function from ASwapchainImage to populate the VkPresentInfoKHR structure with the appropriate information required for presenting the swapchain images to the screen. It takes a reference to a VkPresentInfoKHR structure as a parameter, which should be filled with the necessary details such as the swapchain, image index, and synchronization primitives. Implement this function to ensure that the rendering system can correctly present rendered frames using the swapchain images.
         *
         * @param presentInfo A reference to a VkPresentInfoKHR structure that should be filled with the necessary information for presenting the swapchain images.
         *
         * @note This function is responsible for ensuring that the presentation information is correctly populated to allow the rendering system to present frames using the swapchain images without issues, such as synchronization problems or incorrect image indices. Proper implementation of this function is crucial for the correct functioning of the presentation pipeline in the rendering system.
         */
		void fillPresentInfo(VkPresentInfoKHR &presentInfo) const override;

        /**
         * The OpenXR swapchain handle associated with the swapchain images.
         */
        XrSwapchain _swapchain;

        /**
         * The dimensions (width and height) of the swapchain images, which are determined based on the swapchain creation info and surface capabilities. These dimensions are essential for configuring the rendering pipeline and ensuring that rendered content is correctly sized for presentation.
         *
         * @note The width and height of the swapchain images are typically determined during the initialization of the XrSwapchainImage instance based on the parameters provided in the swapchain creation info and the capabilities of the surface being rendered to. Properly setting these dimensions is crucial for ensuring that rendered content is displayed correctly on the screen without distortion or scaling issues.
         */
        uint32_t _width = 0;

        /**
         * The height of the swapchain images, which is determined based on the swapchain creation info and surface capabilities. This dimension is essential for configuring the rendering pipeline and ensuring that rendered content is correctly sized for presentation.
         *
         * @note The width and height of the swapchain images are typically determined during the initialization of the XrSwapchainImage instance based on the parameters provided in the swapchain creation info and the capabilities of the surface being rendered to. Properly setting these dimensions is crucial for ensuring that rendered content is displayed correctly on the screen without distortion or scaling issues.
         */
        uint32_t _height = 0;

    private:
        /**
         * A vector of Vulkan swapchain image structures (XrSwapchainImageVulkan2KHR) that represent the individual images in the swapchain. These structures contain information about the Vulkan images associated with each swapchain image, such as the image handle, format, and layout. The vector is used to manage and access the Vulkan images for rendering operations and presentation.
         */
        std::vector<XrSwapchainImageVulkan2KHR> _swapchainImages;

        /**
         * A base pointer for the swapchain images, which is used to access the individual swapchain image structures in the vector. This pointer is typically initialized to point to the first element of the _swapchainImages vector and is used for efficient access to the swapchain image information during rendering and presentation operations.
         */
        XrSwapchainImageBaseHeader *_swapchainImagesBase;
    };
} // namespace evan