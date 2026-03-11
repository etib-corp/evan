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

    auto swapchainFormatCount = deviceContext.getDeviceBackend()->countSwapchainFormats();
	auto swapchainFormats =
		deviceContext.getDeviceBackend()->enumerateSwapchainFormats(swapchainFormatCount);
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
        _swapchainImages[swapchain] = std::make_shared<XrSwapchainImage>(properties);
    }
}