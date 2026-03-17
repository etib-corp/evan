/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrSwapchainContext
*/

#include "openxr/XrSwapchainContext.hpp"

evan::XrSwapchainContext::XrSwapchainContext(
    const DeviceContext &deviceContext)
{
    createRenderPass(deviceContext.getDeviceBackend(), deviceContext.getMsaaSamples());

    uint32_t swapchainFormatCount = deviceContext.getDeviceBackend()->countSwapchainFormats();
    auto swapchainFormats = deviceContext.getDeviceBackend()->enumerateSwapchainFormats(swapchainFormatCount);

    _viewsConfigurations = dynamic_cast<XrDeviceBackend *>(deviceContext.getDeviceBackend().get())->enumerateViewConfigurations();
    _views.resize(_viewsConfigurations.size(), { XR_TYPE_VIEW });

	auto swapchainFormat = selectSwapchainFormat(swapchainFormats);

    for (const auto &viewConfig : _viewsConfigurations) {
        XrSwapchainCreateInfo swapchainCreateInfo {};
        swapchainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
        swapchainCreateInfo.arraySize = 1;
        swapchainCreateInfo.format = swapchainFormat;
        swapchainCreateInfo.width = viewConfig.recommendedImageRectWidth;
        swapchainCreateInfo.height = viewConfig.recommendedImageRectHeight;
        swapchainCreateInfo.mipCount = 1;
        swapchainCreateInfo.faceCount = 1;
        swapchainCreateInfo.sampleCount = viewConfig.recommendedSwapchainSampleCount;
        swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;

        XrSwapchain swapchain;
        auto session = dynamic_cast<evan::XrDeviceBackend *>(deviceContext.getDeviceBackend().get())->_session;
        XrResult result = xrCreateSwapchain(session, &swapchainCreateInfo, &swapchain);
        if (result != XR_SUCCESS) {
            std::cerr << "Failed to create swapchain for view configuration with error code: " << result << std::endl;
            continue;
        }
        evan::XrSwapchainImage::CreateXrSwapchainImageProperties properties {
            .swapchain = swapchain,
            .createInfo = swapchainCreateInfo,
            .renderPass = _renderPass,
            .deviceContext = deviceContext
        };
        _swapchainImages.push_back(std::make_shared<XrSwapchainImage>(properties));
    }
}

void evan::XrSwapchainContext::destroy(VkDevice device)
{
    for (const auto &swapchainImage : _swapchainImages) {
        swapchainImage->destroy(device);
    }
    _swapchainImages.clear();
}

VkResult evan::XrSwapchainContext::aquireImage(uint32_t index, VkDevice device, VkSemaphore imageAvailableSemaphore, VkFence inFlightFence, uint32_t &imageIndex)
{
    XrSwapchain swapchain = dynamic_cast<XrSwapchainImage *>(_swapchainImages[index].get())->_swapchain;
    XrSwapchainImageAcquireInfo acquire_info{};
    acquire_info.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
    XrResult result = xrAcquireSwapchainImage(swapchain, &acquire_info, &imageIndex);
    if (result != XR_SUCCESS) {
        std::cerr << "Failed to acquire swapchain image with error code: " << result << std::endl;
        return VK_ERROR_OUT_OF_DATE_KHR;
    }
    return VK_SUCCESS;
}