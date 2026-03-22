/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopSwapchainImage
*/

#pragma once

#include "ASwapchainImage.hpp"

#include "glfw/DesktopBackend.hpp"

namespace evan {
    /**
     * @brief A swapchain image implementation for desktop platforms using GLFW.
     *
     * This class manages the creation and presentation of swapchain images for a GLFW window. It handles the selection of surface formats, present modes, and swap extents based on the capabilities of the physical device and the window properties.
     *
     * The DesktopSwapchainImage class inherits from ASwapchainImage and implements the necessary functionality to create and manage a Vulkan swapchain for a desktop application. It provides methods to destroy the swapchain and fill presentation information for presenting images to the screen.
     *
     * @see ASwapchainImage
     */
    class DesktopSwapchainImage : public ASwapchainImage {
        public:
        /**
         * @brief Constructs a DesktopSwapchainImage instance for a given device context, GLFW window, and render pass.
         * 
         * This constructor initializes the swapchain image by creating a Vulkan swapchain based on the provided device context, GLFW window, and render pass. It selects appropriate surface formats, present modes, and swap extents to ensure optimal performance and compatibility with the rendering pipeline.
         * 
         * @param deviceContext The device context containing the Vulkan device and related resources needed for swapchain creation.
         * @param window The GLFW window for which the swapchain image is being created. The
         * window properties are used to determine the swap extent and other parameters for the swapchain.
         * @param renderpass The Vulkan render pass that will be used with the swapchain images
         * during rendering operations. The render pass defines the attachments and subpasses that will be used when rendering to the swapchain images.
         *
         * @note The constructor performs several steps to create the swapchain, including:
         * - Querying the physical device for swapchain support details.
         * - Choosing appropriate surface formats, present modes, and swap extents based on the capabilities
         * of the physical device and the properties of the GLFW window.
         * - Creating the Vulkan swapchain and associated image views for rendering.
         * - Storing the swapchain handle for later use in presentation and cleanup operations.
         */
        DesktopSwapchainImage(const DeviceContext &deviceContext, GLFWwindow *window, VkRenderPass renderpass);

        ~DesktopSwapchainImage();

        /**
         * @brief Destroys the swapchain image and releases associated resources.
         *
         * This function overrides the pure virtual destroy method from ASwapchainImage to handle the specific logic for destroying the Vulkan swapchain and releasing any resources associated with it. It takes a Vulkan device as a parameter, which is used to perform the necessary cleanup operations for the swapchain.
         *
         * @param device The Vulkan device used to perform the cleanup operations for the swapchain image. This device is typically obtained from the device context and is used to destroy the swapchain and release associated resources.
         *
         * @note This function should ensure that all resources related to the swapchain image are properly released when the image is no longer needed, preventing memory leaks and ensuring efficient resource management in the rendering system.
        */
        void destroy(VkDevice device) override;

        /**
         * @brief Fills the VkPresentInfoKHR structure with the necessary information for presenting the swapchain image.
         *
         * This function overrides the pure virtual fillPresentInfo method from ASwapchainImage to populate the VkPresentInfoKHR structure with the appropriate information required for presenting the swapchain image to the screen. It takes a reference to a VkPresentInfoKHR structure as a parameter, which should be filled with the necessary details such as the swapchain, image index, and synchronization primitives.
         *
         * @param presentInfo A reference to a VkPresentInfoKHR structure that should be filled with the necessary information for presenting the swapchain image. This structure is used by the rendering system to present rendered frames using the swapchain images, and it must be correctly populated to ensure proper presentation behavior.
         *
         * @note This function should handle the specific logic for filling the presentation information for swapchain images in the context of the rendering system, ensuring that all required fields in the VkPresentInfoKHR structure are correctly set based on the swapchain and rendering pipeline configuration.
         */
		void fillPresentInfo(VkPresentInfoKHR &presentInfo) const override;

        /**
         * The Vulkan swapchain associated with this swapchain image. The swapchain is created during the initialization of the DesktopSwapchainImage instance and is used for presenting rendered images to the screen. It is important to properly manage the lifecycle of the swapchain, ensuring that it is destroyed when no longer needed to release associated resources.
         */
        VkSwapchainKHR _swapchain;

        private:
        /**
         * @brief Chooses the appropriate surface format for the swapchain based on the available formats.
         *
         * This function selects the most suitable surface format for the swapchain from the list of available formats provided by the physical device. The selection process typically involves checking for preferred formats (such as VK_FORMAT_B8G8R8A8_SRGB) and ensuring compatibility with the rendering pipeline.
         *
         * @param availableFormats A vector of VkSurfaceFormatKHR structures representing the surface formats supported by the physical device. The function will iterate through this list to find the best match for the swapchain.
         *
         * @return The chosen VkSurfaceFormatKHR structure that best matches the desired surface format for the swapchain. This format will be used during swapchain creation to ensure optimal performance and compatibility with the rendering pipeline.
         */
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats);

        /**
         * @brief Chooses the appropriate present mode for the swapchain based on the available present modes.
         *
         * This function selects the most suitable present mode for the swapchain from the list of available present modes provided by the physical device. The selection process typically involves checking for preferred present modes (such as VK_PRESENT_MODE_MAILBOX_KHR) that offer better performance and reduced latency for presenting rendered images to the screen.
         *
         * @param availablePresentModes A vector of VkPresentModeKHR values representing the present modes supported by the physical device. The function will iterate through this list to find the best match for the swapchain.
         *
         * @return The chosen VkPresentModeKHR value that best matches the desired present mode for the swapchain. This present mode will be used during swapchain creation to ensure optimal performance and presentation behavior for the rendering system.
         */
        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR> &availablePresentModes);

        /**
         * @brief Chooses the appropriate swap extent (width and height) for the swapchain images based on the surface capabilities and GLFW window properties.
         *
         * This function determines the optimal swap extent for the swapchain images by considering the surface capabilities of the physical device and the properties of the GLFW window. The swap extent defines the width and height of the swapchain images in pixels, and it is crucial for ensuring that rendered content is correctly sized for presentation.
         *
         * @param capabilities A VkSurfaceCapabilitiesKHR structure containing the capabilities of the surface, including the current extent and supported extents for the swapchain images. The function will use this information to determine the appropriate swap extent.
         * @param window The GLFW window for which the swapchain images are being created. The properties of the window, such as its width and height, will be used to calculate the swap extent if the current extent in the surface capabilities is not defined (i.e., if it is set to UINT32_MAX).
         *
         * @return The chosen VkExtent2D structure representing the width and height of the swapchain images. This extent will be used during swapchain creation to ensure that rendered content is correctly sized for presentation on the screen.
         */
        VkExtent2D chooseSwapExtent(
            const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);
    };
}
