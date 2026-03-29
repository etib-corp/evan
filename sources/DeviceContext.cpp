/*
** ETIB PROJECT, 2026
** evan
** File description:
** DeviceContext
*/

#include "DeviceContext.hpp"

/**
 * @brief Default debug callback function for Vulkan validation layers.
 *
 * This function is called whenever a validation layer generates a debug
 * message. It outputs the message to the standard error stream.
 *
 * @param messageSeverity Specifies the severity of the message (e.g., verbose,
 * info, warning, or error).
 * @param messageType Specifies the type of the message (e.g., general,
 * validation, or performance).
 * @param pCallbackData Pointer to a structure containing details about the
 * debug message.
 * @param pUserData Pointer to user-defined data passed during the creation of
 * the debug messenger.
 *
 * @return Always returns VK_FALSE, indicating that the Vulkan call that
 * triggered the callback should not be aborted.
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL defaultDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
	(void)messageSeverity;
	(void)messageType;
	(void)pUserData;
	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

evan::DeviceContext::DeviceContext(const IPlatform &platform)
{
	_deviceBackend = platform.createDeviceBackend();

	this->getMaxUsableSampleCount();
	this->setupDebugMessenger();
	this->createGraphicsQueue();
	this->createCommandPool();
}

evan::DeviceContext::~DeviceContext()
{
	vkDestroyCommandPool(_deviceBackend->_device, _commandPool, nullptr);
	// if (enableValidationLayers) {
	// 	vkDestroyDebugUtilsMessengerEXT(_deviceBackend->_VkInstance,
	// _debugMessenger, 									nullptr);
	// }
	_deviceBackend.reset();
}

/////////////
// Getters //
/////////////

VkSampleCountFlagBits evan::DeviceContext::getMsaaSamples() const
{
	return _msaaSamples;
}

std::shared_ptr<evan::ADeviceBackend>
	evan::DeviceContext::getDeviceBackend() const
{
	return _deviceBackend;
}

VkCommandPool evan::DeviceContext::getCommandPool() const
{
	return _commandPool;
}

VkQueue evan::DeviceContext::getGraphicsQueue() const
{
	return _graphicsQueue;
}

void evan::DeviceContext::getMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(_deviceBackend->_physicalDevice,
								  &physicalDeviceProperties);
	VkSampleCountFlags counts =
		physicalDeviceProperties.limits.framebufferColorSampleCounts
		& physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT) {
		_msaaSamples = VK_SAMPLE_COUNT_64_BIT;
		return;
	}
	if (counts & VK_SAMPLE_COUNT_32_BIT) {
		_msaaSamples = VK_SAMPLE_COUNT_32_BIT;
		return;
	}
	if (counts & VK_SAMPLE_COUNT_16_BIT) {
		_msaaSamples = VK_SAMPLE_COUNT_16_BIT;
		return;
	}
	if (counts & VK_SAMPLE_COUNT_8_BIT) {
		_msaaSamples = VK_SAMPLE_COUNT_8_BIT;
		return;
	}
	if (counts & VK_SAMPLE_COUNT_4_BIT) {
		_msaaSamples = VK_SAMPLE_COUNT_4_BIT;
		return;
	}
	if (counts & VK_SAMPLE_COUNT_2_BIT) {
		_msaaSamples = VK_SAMPLE_COUNT_2_BIT;
		return;
	}
	_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
}

/////////////////////
// Private methods //
/////////////////////

void evan::DeviceContext::createCommandPool()
{
	evan::QueueFamilyIndices queueFamilyIndices =
		_deviceBackend->findQueueFamilies();

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex =
		queueFamilyIndices.graphicsFamily.value();
	commandPoolCreateInfo.flags =
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkResult result =
		vkCreateCommandPool(_deviceBackend->_device, &commandPoolCreateInfo,
							nullptr, &_commandPool);
	if (result != VK_SUCCESS) {
		// TODO: replace with proper error handling
		std::cerr << "Failed to create command pool: " << result << std::endl;
		return;
	}
}

void evan::DeviceContext::createGraphicsQueue()
{
	QueueFamilyIndices indices = _deviceBackend->findQueueFamilies();

	vkGetDeviceQueue(_deviceBackend->_device, indices.graphicsFamily.value(), 0,
					 &_graphicsQueue);
}

void evan::DeviceContext::setupDebugMessenger()
{
	if (!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;

	this->populateDebugMessengerCreateInfo(createInfo, defaultDebugCallback);
	if (this->createDebugUtilsMessengerEXT(
			_deviceBackend->_VkInstance, &createInfo, nullptr, &_debugMessenger)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to set up debug messenger !");
	}
}

void evan::DeviceContext::populateDebugMessengerCreateInfo(
	VkDebugUtilsMessengerCreateInfoEXT &createInfo,
	PFN_vkDebugUtilsMessengerCallbackEXT debugCallback)
{
	createInfo		 = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

VkResult evan::DeviceContext::createDebugUtilsMessengerEXT(
	VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
	const VkAllocationCallbacks *pAllocator,
	VkDebugUtilsMessengerEXT *pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
