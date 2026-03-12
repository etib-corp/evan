/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrSwapchainImage
*/

#include "openxr/XrSwapchainImage.hpp"

evan::XrSwapchainImage::XrSwapchainImage(const CreateXrSwapchainImageProperties &properties)
{
    uint32_t swapchainImageCount = 0;
    xrEnumerateSwapchainImages(properties.swapchain, 0, &swapchainImageCount, nullptr);

    _format = static_cast<VkFormat>(properties.createInfo.format);
    _extent = { properties.createInfo.width, properties.createInfo.height };

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

    xrEnumerateSwapchainImages(properties.swapchain, swapchainImageCount, &swapchainImageCount, _swapchainImagesBase);

    for (auto &image : _swapchainImages) {
        image.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
    }
    for (size_t i = 0; i < _swapchainImages.size(); ++i) {
         _imageViews[i] = properties.deviceContext.getDeviceBackend()->createImageView(_swapchainImages[i].image, _format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }

    this->createColorResources(*properties.deviceContext.getDeviceBackend(), properties.deviceContext.getMsaaSamples());
    this->createDepthResources(properties.deviceContext);
    this->createFramebuffers(properties.deviceContext.getDeviceBackend()->_device, properties.renderPass);
}
