/*
** ETIB PROJECT, 2026
** evan
** File description:
** Frame
*/

#include "evan/Frame.hpp"

evan::Frame::Frame(VkCommandPool commandPool,
				   const ADeviceBackend &deviceBackend)
{
	this->getLogger().info()
		<< "Creating frame with command pool and device backend...";

	this->createCommandBuffer(deviceBackend._device, commandPool);
	this->createSyncObjects(deviceBackend._device);
	this->createUniformBuffer(deviceBackend);

	this->getLogger().info() << "Frame created successfully.";
}

evan::Frame::~Frame()
{
	this->getLogger().info() << "Destroying frame...";
}

////////////////////
// Public Methods //
////////////////////

void evan::Frame::destroy(VkDevice device)
{
	this->getLogger().info() << "Destroying Vulkan resources for frame...";

	this->getLogger().info()
		<< "Destroying uniform buffer and freeing memory...";
	vkDestroyBuffer(device, _uniformBuffer, nullptr);

	this->getLogger().info() << "Freeing uniform buffer memory...";
	vkFreeMemory(device, _uniformBufferMemory, nullptr);

	this->getLogger().info() << "Destroying synchronization objects...";
	vkDestroySemaphore(device, _render, nullptr);

	this->getLogger().info() << "Destroying image semaphore...";
	vkDestroySemaphore(device, _image, nullptr);

	this->getLogger().info() << "Destroying in-flight fence...";
	vkDestroyFence(device, _inFlight, nullptr);
}

void evan::Frame::resetCommandBuffer()
{
	this->getLogger().info() << "Resetting command buffer for frame...";
	vkResetCommandBuffer(_commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
}

/////////////
// Getters //
/////////////

VkBuffer evan::Frame::getUniformBuffer() const
{
	return _uniformBuffer;
}

/////////////////////
// Private Methods //
/////////////////////

void evan::Frame::createCommandBuffer(VkDevice device,
									  VkCommandPool commandPool)
{
	this->getLogger().info() << "Creating command buffer for frame...";

	VkCommandBufferAllocateInfo allocInfo {};
	allocInfo.sType		  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level		  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	this->getLogger().info()
		<< "Allocating command buffer from command pool...";

	if (vkAllocateCommandBuffers(device, &allocInfo, &_commandBuffer)
		!= VK_SUCCESS) {
		this->getLogger().error()
			<< "Failed to allocate command buffer for frame!";
		return;
	}
	this->getLogger().info() << "Command buffer allocated successfully.";
}

void evan::Frame::createSyncObjects(VkDevice device)
{
	this->getLogger().info() << "Creating synchronization objects for frame...";
	VkSemaphoreCreateInfo semaphoreInfo {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	this->getLogger().info() << "Creating image available semaphore...";

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_image)
			!= VK_SUCCESS
		|| vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_render)
			!= VK_SUCCESS
		|| vkCreateFence(device, &fenceInfo, nullptr, &_inFlight)
			!= VK_SUCCESS) {
		this->getLogger().error()
			<< "Failed to create synchronization objects for frame!";
		return;
	}
	this->getLogger().info() << "Synchronization objects created successfully.";
}

void evan::Frame::createUniformBuffer(const ADeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Creating uniform buffer for frame...";
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	this->getLogger().info()
		<< "Setting up buffer properties for uniform buffer...";
	ADeviceBackend::CreateBufferProperties bufferProperties = {
		._size		 = bufferSize,
		._usage		 = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		._properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		._buffer	   = _uniformBuffer,
		._bufferMemory = _uniformBufferMemory
	};

	this->getLogger().info()
		<< "Creating uniform buffer and allocating memory with properties: "
		<< bufferProperties._size
		<< " bytes, usage flags: " << bufferProperties._usage
		<< ", memory properties: " << bufferProperties._properties << "...";

	deviceBackend.createBuffer(bufferProperties);

	this->getLogger().info() << "Uniform buffer created and memory allocated "
								"successfully. Mapping memory...";
	vkMapMemory(deviceBackend._device, _uniformBufferMemory, 0, bufferSize, 0,
				&_uniformBufferMapped);
}
