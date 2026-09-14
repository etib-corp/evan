/*
** ETIB PROJECT, 2026
** evan
** File description:
** Frame
*/

#include "evan/Frame.hpp"

evan::Frame::Frame(std::shared_ptr<DeviceContext> deviceContext)
	: _deviceContext(deviceContext)
{
	this->getLogger().info()
		<< "Creating frame with command pool and device backend...";

	auto deviceBackend = _deviceContext->getDeviceBackend();
	auto commandPool   = _deviceContext->getCommandPool();

	this->createCommandBuffer(deviceBackend->getDevice(), commandPool);
	this->createSyncObjects(deviceBackend->getDevice());
	this->createUniformBuffer(*deviceBackend);
	this->createInstanceBuffer(*deviceBackend);

	this->getLogger().info() << "Frame created successfully.";
}

evan::Frame::~Frame()
{
	this->getLogger().info() << "Destroying frame...";
	this->cleanup();
}

////////////////////
// Public Methods //
////////////////////

void evan::Frame::destroy(VkDevice device)
{
	(void)device;
	this->cleanup();
}

/////////////////////
// Private Methods //
/////////////////////

void evan::Frame::cleanup()
{
	if (!_deviceContext || !_deviceContext->getDeviceBackend()) {
		return;
	}

	VkDevice device = _deviceContext->getDeviceBackend()->getDevice();

	this->getLogger().info() << "Destroying synchronization objects...";
	for (auto semaphore: _imageAvailable) {
		if (semaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(device, semaphore, nullptr);
		}
	}
	for (auto semaphore: _renderFinished) {
		if (semaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(device, semaphore, nullptr);
		}
	}
	if (_inFlight != VK_NULL_HANDLE) {
		vkDestroyFence(device, _inFlight, nullptr);
		_inFlight = VK_NULL_HANDLE;
	}
	_imageAvailable.clear();
	_renderFinished.clear();

	this->getLogger().info()
		<< "Destroying uniform buffer and freeing memory...";
	if (_uniformBuffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, _uniformBuffer, nullptr);
		_uniformBuffer = VK_NULL_HANDLE;
	}
	if (_uniformBufferMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, _uniformBufferMemory, nullptr);
		_uniformBufferMemory = VK_NULL_HANDLE;
	}

	this->getLogger().info()
		<< "Destroying instance buffer and freeing memory...";
	if (_instanceBuffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, _instanceBuffer, nullptr);
		_instanceBuffer = VK_NULL_HANDLE;
	}
	if (_instanceBufferMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, _instanceBufferMemory, nullptr);
		_instanceBufferMemory = VK_NULL_HANDLE;
	}
	_instanceBufferMapped = nullptr;
}

void evan::Frame::resetCommandBuffer(std::size_t viewSlot)
{
	this->getLogger().info()
		<< "Resetting command buffer for view slot " << viewSlot << "...";
	vkResetCommandBuffer(_commandBuffers[viewSlot],
						 /*VkCommandBufferResetFlagBits*/ 0);
}

/////////////
// Getters //
/////////////

VkCommandBuffer evan::Frame::getCommandBuffer(std::size_t viewSlot) const
{
	return _commandBuffers[viewSlot];
}

VkBuffer evan::Frame::getUniformBuffer() const
{
	return _uniformBuffer;
}

VkDeviceSize evan::Frame::getUniformBufferAlignedSize() const
{
	return _uniformBufferAlignedSize;
}

void *evan::Frame::getUniformBufferMapped(std::size_t viewSlot) const
{
	return static_cast<char *>(_uniformBufferMapped)
		+ viewSlot * _uniformBufferAlignedSize;
}

VkBuffer evan::Frame::getInstanceBuffer() const
{
	return _instanceBuffer;
}

void *evan::Frame::getInstanceBufferMapped(std::size_t viewSlot) const
{
	return static_cast<char *>(_instanceBufferMapped)
		+ viewSlot * _instanceBufferAlignedSize;
}

VkDeviceSize evan::Frame::getInstanceBufferAlignedSize() const
{
	return _instanceBufferAlignedSize;
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
	allocInfo.commandBufferCount = MAX_SWAPCHAINS;

	this->getLogger().info()
		<< "Allocating " << MAX_SWAPCHAINS
		<< " command buffers from command pool...";

	_commandBuffers.resize(MAX_SWAPCHAINS);
	if (vkAllocateCommandBuffers(device, &allocInfo, _commandBuffers.data())
		!= VK_SUCCESS) {
		this->getLogger().error()
			<< "Failed to allocate command buffers for frame!";
		_commandBuffers.clear();
		return;
	}
	this->getLogger().info() << "Command buffers allocated successfully.";
}

void evan::Frame::createSyncObjects(VkDevice device)
{
	this->getLogger().info() << "Creating synchronization objects for frame...";
	VkSemaphoreCreateInfo semaphoreInfo {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	_imageAvailable.resize(MAX_SWAPCHAINS);
	_renderFinished.resize(MAX_SWAPCHAINS);

	for (int i = 0; i < MAX_SWAPCHAINS; ++i) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr,
							  &_imageAvailable[i])
				!= VK_SUCCESS
			|| vkCreateSemaphore(device, &semaphoreInfo, nullptr,
								 &_renderFinished[i])
				!= VK_SUCCESS) {
			this->getLogger().error()
				<< "Failed to create synchronization objects for frame!";
			return;
		}
	}

	if (vkCreateFence(device, &fenceInfo, nullptr, &_inFlight) != VK_SUCCESS) {
		this->getLogger().error()
			<< "Failed to create in-flight fence for frame!";
		return;
	}
	this->getLogger().info() << "Synchronization objects created successfully.";
}

void evan::Frame::createUniformBuffer(const ADeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Creating uniform buffer for frame...";

	VkPhysicalDeviceProperties deviceProperties {};
	vkGetPhysicalDeviceProperties(deviceBackend.getPhysicalDevice(),
								  &deviceProperties);
	const VkDeviceSize minAlignment =
		deviceProperties.limits.minUniformBufferOffsetAlignment;

	const auto alignUp = [](VkDeviceSize value, VkDeviceSize alignment) {
		return (value + alignment - 1) & ~(alignment - 1);
	};
	_uniformBufferAlignedSize =
		alignUp(sizeof(UniformBufferObject), minAlignment);
	const VkDeviceSize bufferSize =
		_uniformBufferAlignedSize * MAX_SWAPCHAINS;

	this->getLogger().info()
		<< "Uniform buffer: " << MAX_SWAPCHAINS << " view slot(s) of "
		<< _uniformBufferAlignedSize << " bytes (" << bufferSize
		<< " total).";

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
	vkMapMemory(deviceBackend.getDevice(), _uniformBufferMemory, 0, bufferSize,
				0, &_uniformBufferMapped);
}

void evan::Frame::createInstanceBuffer(const ADeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Creating instance buffer for frame...";

	VkPhysicalDeviceProperties deviceProperties {};
	vkGetPhysicalDeviceProperties(deviceBackend.getPhysicalDevice(),
								  &deviceProperties);
	const VkDeviceSize minAlignment =
		deviceProperties.limits.minUniformBufferOffsetAlignment;

	const auto alignUp = [](VkDeviceSize value, VkDeviceSize alignment) {
		return (value + alignment - 1) & ~(alignment - 1);
	};
	const VkDeviceSize instanceStride = sizeof(glm::mat4);
	_instanceBufferAlignedSize =
		alignUp(instanceStride * MAX_INSTANCES_PER_VIEW, minAlignment);
	const VkDeviceSize bufferSize =
		_instanceBufferAlignedSize * MAX_SWAPCHAINS;

	this->getLogger().info()
		<< "Instance buffer: " << MAX_SWAPCHAINS << " view slot(s) of "
		<< _instanceBufferAlignedSize << " bytes (" << bufferSize
		<< " total).";

	ADeviceBackend::CreateBufferProperties bufferProperties = {
		._size		 = bufferSize,
		._usage		 = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		._properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		._buffer	   = _instanceBuffer,
		._bufferMemory = _instanceBufferMemory
	};

	deviceBackend.createBuffer(bufferProperties);

	this->getLogger().info() << "Instance buffer created and memory allocated "
								"successfully. Mapping memory...";
	vkMapMemory(deviceBackend.getDevice(), _instanceBufferMemory, 0,
				bufferSize, 0, &_instanceBufferMapped);
}
