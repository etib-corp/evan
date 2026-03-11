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

    this->createImageViews(properties.deviceContext.getDeviceBackend()->_device);

    for (auto &image : _swapchainImages) {
        image.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
    }
    for (size_t i = 0; i < _swapchainImages.size(); ++i) {
        this->createImageView(_swapchainImages[i].image, _format, VK_IMAGE_ASPECT_COLOR_BIT, properties.deviceContext.getDeviceBackend()->_device, 1);
    }

    this->createColorResources(*properties.deviceContext.getDeviceBackend(), properties.deviceContext.getMsaaSamples());
    this->createDepthResources(properties.deviceContext);
    this->createFramebuffers(properties.deviceContext.getDeviceBackend()->_device, properties.renderPass);
    this->createCommandBuffer(properties.deviceContext);
    this->createSynchronizationObjects(properties.deviceContext);
}

void evan::XrSwapchainImage::createCommandBuffer(const DeviceContext &deviceContext)
{
    _commandBuffers.resize(_swapchainImages.size());

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = deviceContext.getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

    if (vkAllocateCommandBuffers(deviceContext.getDeviceBackend()->_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers for swapchain images");
    }
}

void evan::XrSwapchainImage::createSynchronizationObjects(const DeviceContext &deviceContext)
{
    _inFlightFences.resize(_swapchainImages.size());
    _imageAvailableFences.resize(_swapchainImages.size());

    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < _swapchainImages.size(); ++i) {
        if (vkCreateFence(deviceContext.getDeviceBackend()->_device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS ||
            vkCreateFence(deviceContext.getDeviceBackend()->_device, &fenceInfo, nullptr, &_imageAvailableFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization objects for swapchain images");
        }
    }
}