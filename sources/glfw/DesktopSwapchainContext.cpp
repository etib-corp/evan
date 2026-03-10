/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopSwapchainContext
*/

#include "glfw/DesktopSwapchainContext.hpp"

evan::DesktopSwapchainContext::DesktopSwapchainContext(const DeviceContext &deviceContext, GLFWwindow *window)
{
    this->createRenderPass(deviceContext.getDeviceBackend(), deviceContext.getMsaaSamples());
    this->_swapchainImage = std::make_shared<DesktopSwapchainImage>(deviceContext, window, _renderPass);
}

evan::DesktopSwapchainContext::~DesktopSwapchainContext()
{
}
