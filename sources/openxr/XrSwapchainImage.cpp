/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrSwapchainImage
*/

#include "openxr/XrSwapchainImage.hpp"

evan::XrSwapchainImage::XrSwapchainImage(const DeviceContext &deviceContext, XrSwapchain swapchain, XrSwapchainCreateInfo createInfo)
{
    uint32_t swapchainImageCount = 0;
    xrEnumerateSwapchainImages(swapchain, 0, &swapchainImageCount, nullptr);

    _format = static_cast<VkFormat>(createInfo.format);
    _extent = { createInfo.width, createInfo.height };

    _swapchainImages.resize(swapchainImageCount, { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN2_KHR });
    _imageViews.resize(swapchainImageCount);
    _framebuffers.resize(swapchainImageCount);

    _viewport.x = 0.0f;
    _viewport.y = static_cast<float>(_extent.height);
    _viewport.width = static_cast<float>(_extent.width);
    _viewport.height = static_cast<float>(_extent.height);
    _viewport.minDepth = 0.0f;
    _viewport.maxDepth = 1.0f;

    _scissor.extent = _extent;

    _swapchainImagesBase = reinterpret_cast<XrSwapchainImageBaseHeader *>(&_swapchainImages[0]);

    xrEnumerateSwapchainImages(swapchain, swapchainImageCount, &swapchainImageCount, _swapchainImagesBase);

    this->createImageViews(deviceContext.getDeviceBackend()->_device);

    for (auto &image : _swapchainImages) {
        image.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;
        evan::ADeviceBackend::CreateImageProperties imageProperties = {
            ._logicalDevice = deviceContext.getDeviceBackend()->_device,
            ._physicalDevice = deviceContext.getDeviceBackend()->_physicalDevice,
            ._width = createInfo.width,
            ._height = createInfo.height,
            ._mipLevels = 1,
            ._numSamples = deviceContext.getMsaaSamples(),
            ._format = _format,
            ._tiling = VK_IMAGE_TILING_OPTIMAL,
            ._usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            ._properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            ._image = image.image,
            ._imageMemory = imageMemory
        };
        deviceContext.getDeviceBackend()->createImage(imageProperties);
    }
}