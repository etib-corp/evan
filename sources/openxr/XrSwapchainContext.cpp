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

    for (const auto &viewConfig : _viewsConfigurations) {

    }
}