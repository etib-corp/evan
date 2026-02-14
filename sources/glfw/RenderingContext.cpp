/*
** ETIB PROJECT, 2025
** evan
** File description:
** RenderingContext
*/

#include "glfw/RenderingContext.hpp"

////////////////////
// Public methods //
////////////////////

evan::glfw::RenderingContext::RenderingContext(VkInstance instance,
											   GLFWwindow *window)
{
	this->createSurface(instance, window);
	this->pickPhysicalDevice(instance);
	this->createLogicalDevice();
	this->createCommandPool();
	this->createVertexBuffer();
	this->createIndexBuffer();
	this->createCommandBuffers();
	this->createSyncObjects();

	// Initialize VulkanContext (used to setup the rest of the engine)
	_vulkanContext				   = std::make_shared<VulkanContext>();
	_vulkanContext->logicalDevice  = _logicalDevice;
	_vulkanContext->physicalDevice = _physicalDevice;
	_vulkanContext->graphicsQueue  = _graphicsQueue;
	_vulkanContext->commandPool	   = _commandPool;
	_vulkanContext->graphicsQueueFamilyIndex =
		Utils::findQueueFamilies(_physicalDevice, _surface)
			.graphicsFamily.value();
	_vulkanContext->surface		= _surface;
	_vulkanContext->msaaSamples = _msaaSamples;
}

evan::glfw::RenderingContext::~RenderingContext()
{
}

///////////////////////
// Protected methods //
///////////////////////

void evan::glfw::RenderingContext::createSurface(VkInstance instance,
												 GLFWwindow *window)
{
#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR createInfo {};

	createInfo.sType	 = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd		 = glfwGetWin32Window(window);
	createInfo.hinstance = GetModuleHandle(nullptr);

	if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &_surface)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}
#else
	if (instance == VK_NULL_HANDLE) {
		throw std::runtime_error("Vulkan instance is NULL!");
	}
	if (glfwCreateWindowSurface(instance, window, nullptr, &_surface)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}
#endif
}

void evan::glfw::RenderingContext::pickPhysicalDevice(VkInstance instance)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support !");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto &device: devices) {
		if (Utils::isDeviceSuitable(device, _surface, deviceExtensions)) {
			_physicalDevice = device;
			_msaaSamples	= this->getMaxUsableSampleCount();
			break;
		}
	}

	if (_physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void evan::glfw::RenderingContext::createLogicalDevice()
{
	Utils::QueueFamilyIndices indices =
		Utils::findQueueFamilies(_physicalDevice, _surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),
											   indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily: uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount		 = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;

	VkDeviceCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount =
		static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount =
		static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount =
			static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_logicalDevice)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(_logicalDevice, indices.graphicsFamily.value(), 0,
					 &_graphicsQueue);
	vkGetDeviceQueue(_logicalDevice, indices.presentFamily.value(), 0,
					 &_presentQueue);
}

void evan::glfw::RenderingContext::createCommandPool()
{
	Utils::QueueFamilyIndices queueFamilyIndices =
		Utils::findQueueFamilies(_physicalDevice, _surface);

	VkCommandPoolCreateInfo poolInfo {};
	poolInfo.sType			  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags			  = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(_logicalDevice, &poolInfo, nullptr, &_commandPool)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}
}

void evan::glfw::RenderingContext::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	if (bufferSize == 0) {
		return;
	}

	Utils::CreateBufferProperties stagingBufferProperties = {
		._logicalDevice	 = _logicalDevice,
		._physicalDevice = _physicalDevice,
		._size			 = bufferSize,
		._usage			 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		._properties	 = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		._buffer	   = stagingBuffer,
		._bufferMemory = stagingBufferMemory
	};

	void *data;
	vkMapMemory(_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, _vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(_logicalDevice, stagingBufferMemory);

	Utils::CreateBufferProperties vertexBufferProperties = {
		._logicalDevice	 = _logicalDevice,
		._physicalDevice = _physicalDevice,
		._size			 = bufferSize,
		._usage			 = VK_BUFFER_USAGE_TRANSFER_DST_BIT
			| VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		._properties   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._buffer	   = _vertexBuffer,
		._bufferMemory = _vertexBufferMemory
	};
	Utils::createBuffer(vertexBufferProperties);

	Utils::CopyBufferProperties copyBufferProperties = {
		._logicalDevice = _logicalDevice,
		._commandPool	= _commandPool,
		._graphicsQueue = _graphicsQueue,
		._srcBuffer		= stagingBuffer,
		._dstBuffer		= _vertexBuffer,
		._size			= bufferSize
	};
	Utils::copyBuffer(copyBufferProperties);

	vkDestroyBuffer(_logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(_logicalDevice, stagingBufferMemory, nullptr);
}

void evan::glfw::RenderingContext::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();

	if (bufferSize == 0) {
		return;
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Utils::CreateBufferProperties stagingBufferProperties = {
		._logicalDevice	 = _logicalDevice,
		._physicalDevice = _physicalDevice,
		._size			 = bufferSize,
		._usage			 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		._properties	 = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		._buffer	   = stagingBuffer,
		._bufferMemory = stagingBufferMemory
	};
	Utils::createBuffer(stagingBufferProperties);

	void *data;
	vkMapMemory(_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, _indices.data(), (size_t)bufferSize);
	vkUnmapMemory(_logicalDevice, stagingBufferMemory);

	Utils::CreateBufferProperties indexBufferProperties = {
		._logicalDevice	 = _logicalDevice,
		._physicalDevice = _physicalDevice,
		._size			 = bufferSize,
		._usage =
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		._properties   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._buffer	   = _indexBuffer,
		._bufferMemory = _indexBufferMemory
	};
	Utils::createBuffer(indexBufferProperties);

	Utils::CopyBufferProperties copyBufferProperties = {
		._logicalDevice = _logicalDevice,
		._commandPool	= _commandPool,
		._graphicsQueue = _graphicsQueue,
		._srcBuffer		= stagingBuffer,
		._dstBuffer		= _indexBuffer,
		._size			= bufferSize
	};
	Utils::copyBuffer(copyBufferProperties);

	vkDestroyBuffer(_logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(_logicalDevice, stagingBufferMemory, nullptr);
}

void evan::glfw::RenderingContext::createCommandBuffers()
{
	_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo {};
	allocInfo.sType		  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _commandPool;
	allocInfo.level		  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)_commandBuffers.size();

	if (vkAllocateCommandBuffers(_logicalDevice, &allocInfo,
								 _commandBuffers.data())
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

void evan::glfw::RenderingContext::createSyncObjects()
{
	_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(_logicalDevice, &semaphoreInfo, nullptr,
							  &_imageAvailableSemaphores[i])
				!= VK_SUCCESS
			|| vkCreateSemaphore(_logicalDevice, &semaphoreInfo, nullptr,
								 &_renderFinishedSemaphores[i])
				!= VK_SUCCESS
			|| vkCreateFence(_logicalDevice, &fenceInfo, nullptr,
							 &_inFlightFences[i])
				!= VK_SUCCESS) {
			throw std::runtime_error(
				"Failed to create synchronization objects for a frame !");
		}
	}
}

/////////////////////
// Private methods //
/////////////////////

VkSampleCountFlagBits evan::glfw::RenderingContext::getMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(_physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts =
		physicalDeviceProperties.limits.framebufferColorSampleCounts
		& physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) {
		return VK_SAMPLE_COUNT_64_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_32_BIT) {
		return VK_SAMPLE_COUNT_32_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_16_BIT) {
		return VK_SAMPLE_COUNT_16_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_8_BIT) {
		return VK_SAMPLE_COUNT_8_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_4_BIT) {
		return VK_SAMPLE_COUNT_4_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_2_BIT) {
		return VK_SAMPLE_COUNT_2_BIT;
	}

	return VK_SAMPLE_COUNT_1_BIT;
}
