/*
** ETIB PROJECT, 2026
** evan
** File description:
** Frame
*/

#include "Frame.hpp"

evan::Frame::Frame(VkCommandPool commandPool, const ADeviceBackend &deviceBackend)
{
    this->createCommandBuffer(deviceBackend._device, commandPool);
    this->createSyncObjects(deviceBackend._device);
    this->createUniformBuffer(deviceBackend);
}

evan::Frame::~Frame()
{
}

void evan::Frame::destroy(VkDevice device)
{
    vkDestroyBuffer(device, _uniformBuffer, nullptr);
    vkFreeMemory(device, _uniformBufferMemory, nullptr);
    vkDestroySemaphore(device, _render, nullptr);
    vkDestroySemaphore(device, _image, nullptr);
    vkDestroyFence(device, _inFlight, nullptr);
}

void evan::Frame::createCommandBuffer(VkDevice device, VkCommandPool commandPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void evan::Frame::createSyncObjects(VkDevice device)
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_image) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_render) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &_inFlight) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
}

void evan::Frame::createUniformBuffer(const ADeviceBackend &deviceBackend)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    ADeviceBackend::CreateBufferProperties bufferProperties = {
			._logicalDevice	 = deviceBackend._device,
			._physicalDevice = deviceBackend._physicalDevice,
			._size			 = bufferSize,
			._usage			 = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			._properties	 = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			._buffer	   = _uniformBuffer,
			._bufferMemory = _uniformBufferMemory
		};

    deviceBackend.createBuffer(bufferProperties);
    vkMapMemory(deviceBackend._device, _uniformBufferMemory, 0, bufferSize, 0,
                &_uniformBufferMapped);
}

VkBuffer evan::Frame::getUniformBuffer() const
{
    return _uniformBuffer;
}

void evan::Frame::resetCommandBuffer()
{
    vkResetCommandBuffer(_commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
}
