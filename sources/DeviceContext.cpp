/*
** ETIB PROJECT, 2026
** evan
** File description:
** DeviceContext
*/

#include "evan/DeviceContext.hpp"

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
	static std::unique_ptr<utility::logging::Logger> logger = nullptr;
	std::string_view message;
	if (!logger) {
		logger = std::make_unique<utility::logging::DefaultLogger>(
			"VulkanValidation");
	}

	switch (messageType) {
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			message = "General";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			message = "Validation";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			message = "Performance";
			break;
		default:
			message = "Unknown";
			break;
	}

	switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			logger->error()
				<< "[" << message << "] " << pCallbackData->pMessage;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			logger->warning()
				<< "[" << message << "] " << pCallbackData->pMessage;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			logger->info() << "[" << message << "] " << pCallbackData->pMessage;
			break;
		default:
			logger->debug()
				<< "[" << message << "] " << pCallbackData->pMessage;
			break;
	}
	return VK_FALSE;
}

evan::DeviceContext::DeviceContext(const IPlatform &platform)
{
	this->getLogger().info() << "Initializing device context...";

	_deviceBackend = platform.createDeviceBackend();

	this->getMaxUsableSampleCount();
	if (enableValidationLayers) {
		this->setupDebugMessenger();
	}
	this->createGraphicsQueue();
	this->createCommandPool();
}

evan::DeviceContext::~DeviceContext()
{
	this->getLogger().info() << "Cleaning up device context...";
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
	this->getLogger().info()
		<< "Determining maximum usable sample count for MSAA...";

	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(_deviceBackend->_physicalDevice,
								  &physicalDeviceProperties);
	VkSampleCountFlags counts =
		physicalDeviceProperties.limits.framebufferColorSampleCounts
		& physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	this->getLogger().info()
		<< "Supported sample counts: " << counts << " bits";

	if (counts & VK_SAMPLE_COUNT_64_BIT) {
		this->getLogger().info() << "Using 64 samples for MSAA.";
		_msaaSamples = VK_SAMPLE_COUNT_64_BIT;
		return;
	}
	if (counts & VK_SAMPLE_COUNT_32_BIT) {
		this->getLogger().info() << "Using 32 samples for MSAA.";
		_msaaSamples = VK_SAMPLE_COUNT_32_BIT;
		return;
	}
	if (counts & VK_SAMPLE_COUNT_16_BIT) {
		this->getLogger().info() << "Using 16 samples for MSAA.";
		_msaaSamples = VK_SAMPLE_COUNT_16_BIT;
		return;
	}
	if (counts & VK_SAMPLE_COUNT_8_BIT) {
		this->getLogger().info() << "Using 8 samples for MSAA.";
		_msaaSamples = VK_SAMPLE_COUNT_8_BIT;
		return;
	}
	if (counts & VK_SAMPLE_COUNT_4_BIT) {
		this->getLogger().info() << "Using 4 samples for MSAA.";
		_msaaSamples = VK_SAMPLE_COUNT_4_BIT;
		return;
	}
	if (counts & VK_SAMPLE_COUNT_2_BIT) {
		this->getLogger().info() << "Using 2 samples for MSAA.";
		_msaaSamples = VK_SAMPLE_COUNT_2_BIT;
		return;
	}
	this->getLogger().info() << "Using 1 sample for MSAA (no multisampling).";
	_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
}

/////////////////////
// Private methods //
/////////////////////

void evan::DeviceContext::createCommandPool()
{
	this->getLogger().info() << "Creating command pool...";

	evan::QueueFamilyIndices queueFamilyIndices =
		_deviceBackend->findQueueFamilies();

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex =
		queueFamilyIndices.graphicsFamily.value();
	commandPoolCreateInfo.flags =
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	this->getLogger().info()
		<< "Command poolCreateInfo initialized with queue family index: "
		<< queueFamilyIndices.graphicsFamily.value();

	VkResult result =
		vkCreateCommandPool(_deviceBackend->_device, &commandPoolCreateInfo,
							nullptr, &_commandPool);
	if (result != VK_SUCCESS) {
		this->getLogger().error()
			<< "Failed to create command pool: " << result;
		return;
	}
}

void evan::DeviceContext::createGraphicsQueue()
{
	this->getLogger().info() << "Retrieving graphics queue...";

	QueueFamilyIndices indices = _deviceBackend->findQueueFamilies();

	this->getLogger().info()
		<< "Graphics queue family index: " << indices.graphicsFamily.value();
	vkGetDeviceQueue(_deviceBackend->_device, indices.graphicsFamily.value(), 0,
					 &_graphicsQueue);
}

bool evan::DeviceContext::checkDebugUtilsSupport(VkInstance instance)
{
	this->getLogger().info() << "Checking for VK_EXT_debug_utils support...";

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
										   extensions.data());

	this->getLogger().info() << "Available instance extensions:";

	for (const auto &extension: extensions) {
		this->getLogger().info()
			<< "  " << std::string(extension.extensionName);
		if (strcmp(extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
			== 0) {
			this->getLogger().info() << "VK_EXT_debug_utils is supported!";
			return true;
		}
	}
	this->getLogger().warning() << "VK_EXT_debug_utils is not supported!";
	return false;
}

void evan::DeviceContext::setupDebugMessenger()
{
	this->getLogger().info() << "Setting up Vulkan debug messenger...";
	if (!enableValidationLayers)
		this->getLogger().warning() << "Validation layers are disabled, "
									   "skipping debug messenger setup.";
	return;

	if (!checkDebugUtilsSupport(_deviceBackend->_VkInstance)) {
		this->getLogger().warning() << "Debug utils extension not supported, "
									   "skipping debug messenger setup.";
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo;

	this->populateDebugMessengerCreateInfo(createInfo, defaultDebugCallback);
	if (this->createDebugUtilsMessengerEXT(
			_deviceBackend->_VkInstance, &createInfo, nullptr, &_debugMessenger)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to set up debug messenger!";
		return;
	}
	this->getLogger().info() << "Debug messenger set up successfully.";
}

void evan::DeviceContext::populateDebugMessengerCreateInfo(
	VkDebugUtilsMessengerCreateInfoEXT &createInfo,
	PFN_vkDebugUtilsMessengerCallbackEXT debugCallback)
{
	this->getLogger().info() << "Populating debug messenger create info...";

	createInfo		 = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;

	this->getLogger().info()
		<< "Debug messenger create info populated with callback: "
		<< reinterpret_cast<uintptr_t>(debugCallback)
		<< " and message severity: " << createInfo.messageSeverity
		<< " and message type: " << createInfo.messageType;
}

VkResult evan::DeviceContext::createDebugUtilsMessengerEXT(
	VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
	const VkAllocationCallbacks *pAllocator,
	VkDebugUtilsMessengerEXT *pDebugMessenger)
{
	this->getLogger().info() << "Creating debug utils messenger...";

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");

	this->getLogger().info()
		<< "vkGetInstanceProcAddr returned function pointer: "
		<< reinterpret_cast<uintptr_t>(func);
	if (func != nullptr) {
		this->getLogger().info() << "Creating debug messenger using "
									"vkCreateDebugUtilsMessengerEXT...";
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		this->getLogger().error() << "vkCreateDebugUtilsMessengerEXT not "
									 "found, cannot create debug messenger.";
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
