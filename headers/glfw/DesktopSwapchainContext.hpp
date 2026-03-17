/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopSwapchainContext
*/

#pragma once

#include "ASwapchainContext.hpp"

#include "glfw/DesktopSwapchainImage.hpp"

#include <memory>

namespace evan {
    class DeviceContext;

    class DesktopSwapchainContext : public ASwapchainContext {
        public:
        DesktopSwapchainContext(const DeviceContext &deviceContext, GLFWwindow *window);
        ~DesktopSwapchainContext();

        void destroy(VkDevice device) override;

        VkResult aquireImage(uint32_t index, VkDevice device, VkSemaphore imageAvailableSemaphore, VkFence inFlightFence, uint32_t &imageIndex) override;

		void waitForImage(uint32_t index) override {}


    };
}
