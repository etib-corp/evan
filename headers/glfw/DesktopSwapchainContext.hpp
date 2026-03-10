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
    class DesktopSwapchainContext : public ASwapchainContext {
        public:
        DesktopSwapchainContext(const DeviceContext &deviceContext, GLFWwindow *window);
        ~DesktopSwapchainContext();

        private:
        std::shared_ptr<DesktopSwapchainImage> _swapchainImage;

    };
}
