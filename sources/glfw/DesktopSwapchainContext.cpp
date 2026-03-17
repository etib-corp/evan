/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopSwapchainContext
*/

#include "glfw/DesktopSwapchainContext.hpp"

#include "DeviceContext.hpp"

evan::DesktopSwapchainContext::DesktopSwapchainContext(const DeviceContext &deviceContext, GLFWwindow *window)
{
    this->createRenderPass(deviceContext.getDeviceBackend(), deviceContext.getMsaaSamples());
    _swapchainImages.push_back(std::make_shared<DesktopSwapchainImage>(deviceContext, window, _renderPass));
}

evan::DesktopSwapchainContext::~DesktopSwapchainContext()
{
}

void evan::DesktopSwapchainContext::destroy(VkDevice device)
{
    vkDestroyRenderPass(device, _renderPass, nullptr);
    for (const auto &swapchainImage : _swapchainImages) {
        swapchainImage->destroy(device);
    }
}

VkResult evan::DesktopSwapchainContext::aquireImage(uint32_t index, VkDevice device, VkSemaphore imageAvailableSemaphore, VkFence inFlightFence, uint32_t &imageIndex)
{
    VkSwapchainKHR swapchain = dynamic_cast<DesktopSwapchainImage*>(_swapchainImages[index].get())->_swapchain;

    return vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore, inFlightFence, &imageIndex);
}
