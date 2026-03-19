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
    class DesktopSwapchainImage : public ASwapchainImage {
        public:
        DesktopSwapchainImage(const DeviceContext &deviceContext, GLFWwindow *window, VkRenderPass renderpass);
        ~DesktopSwapchainImage();

        VkSwapchainKHR _swapchain;

        void destroy(VkDevice device) override;

		void fillPresentInfo(VkPresentInfoKHR &presentInfo) const override;

        private:

            VkSurfaceFormatKHR chooseSwapSurfaceFormat(
	            const std::vector<VkSurfaceFormatKHR> &availableFormats);
            VkPresentModeKHR chooseSwapPresentMode(
	            const std::vector<VkPresentModeKHR> &availablePresentModes);
            VkExtent2D chooseSwapExtent(
	            const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);
    };
}
