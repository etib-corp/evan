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

VkResult evan::DesktopSwapchainContext::aquireImage(VkDevice device, VkSemaphore imageAvailableSemaphore, VkFence inFlightFence, uint32_t &imageIndex)
{
    return vkAcquireNextImageKHR(device, _swapchainImage->_swapchain, UINT64_MAX, imageAvailableSemaphore, inFlightFence, &imageIndex);
}
