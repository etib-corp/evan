/*
** ETIB PROJECT, 2026
** evan
** File description:
** DeviceContext
*/

#include "evan/DeviceContext.hpp"

#include <cstdlib>
#include <optional>

namespace
{
	/**
	 * @brief Sample counts the engine is willing to use.
	 *
	 * Capped at 4x: 8x and above cost far more bandwidth than the edge quality
	 * they add on the scenes the engine targets. Raise this mask to allow
	 * them.
	 */
	constexpr VkSampleCountFlags kSupportedSampleCountMask =
		VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT;

	/**
	 * @brief Sample count used when nothing asks for multisampling.
	 *
	 * A single sample means no MSAA at all: no multisampled color image, no
	 * resolve pass, and no multiplied color or depth bandwidth.
	 */
	constexpr VkSampleCountFlagBits kDefaultMsaaSamples = VK_SAMPLE_COUNT_1_BIT;

	/**
	 * @brief Environment variable overriding the MSAA sample count.
	 *
	 * Set it to 1, 2 or 4 to force a sample count, which makes the cost of
	 * multisampling measurable without rebuilding the engine.
	 */
	constexpr const char *kMsaaSamplesVariable = "EVAN_MSAA";

	/**
	 * @brief Reads the MSAA sample count override from the environment.
	 *
	 * @return The requested sample count, or std::nullopt when the variable is
	 * unset, empty, or holds a value the engine does not support.
	 */
	std::optional<VkSampleCountFlagBits> msaaSamplesFromEnvironment()
	{
		const char *value = std::getenv(kMsaaSamplesVariable);
		if (value == nullptr || value[0] == '\0') {
			return std::nullopt;
		}

		switch (std::atoi(value)) {
			case 1:
				return VK_SAMPLE_COUNT_1_BIT;
			case 2:
				return VK_SAMPLE_COUNT_2_BIT;
			case 4:
				return VK_SAMPLE_COUNT_4_BIT;
			default:
				return std::nullopt;
		}
	}
}	 // namespace

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

	// Multisampling is off by default: an application, or the EVAN_MSAA
	// environment variable, has to ask for it.
	this->setMsaaSamples(kDefaultMsaaSamples);
	if (enableValidationLayers) {
		this->setupDebugMessenger();
	}
	this->createGraphicsQueue();
	this->createCommandPool();

	QueueFamilyIndices indices = _deviceBackend->findQueueFamilies();
	_transferManager.init(_deviceBackend->getDevice(),
						  _deviceBackend->getPhysicalDevice(), _graphicsQueue,
						  indices.graphicsFamily.value());

	this->getLogger().info() << "Creating pipeline cache...";
	_pipelineCache.init(_deviceBackend->getDevice(),
						_deviceBackend->getPhysicalDevice(),
						platform.getPipelineCachePath());
}

evan::DeviceContext::~DeviceContext()
{
	this->getLogger().info() << "Cleaning up device context...";
	_transferManager.destroy();
	vkDestroyCommandPool(_deviceBackend->getDevice(), _commandPool, nullptr);
	if (enableValidationLayers && _debugMessenger != VK_NULL_HANDLE) {
		this->destroyDebugUtilsMessengerEXT(_deviceBackend->getInstance(),
											_debugMessenger);
	}
	// The pipelines created with the cache are already destroyed by the
	// renderer at this point, so the blob can be flushed to disk before the
	// cache and the device go away.
	this->getLogger().info() << "Persisting pipeline cache...";
	_pipelineCache.persist();
	_pipelineCache.destroy(_deviceBackend->getDevice());
	_deviceBackend.reset();
}

/////////////
// Getters //
/////////////

VkSampleCountFlagBits evan::DeviceContext::getMsaaSamples() const
{
	return _msaaSamples;
}

VkSampleCountFlagBits evan::DeviceContext::getMaxSupportedMsaaSamples() const
{
	const VkSampleCountFlags counts =
		this->getFramebufferSampleCounts() & kSupportedSampleCountMask;

	if (counts & VK_SAMPLE_COUNT_4_BIT) {
		return VK_SAMPLE_COUNT_4_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_2_BIT) {
		return VK_SAMPLE_COUNT_2_BIT;
	}
	return VK_SAMPLE_COUNT_1_BIT;
}

void evan::DeviceContext::setMsaaSamples(VkSampleCountFlagBits samples)
{
	this->getLogger().info() << "Overriding MSAA sample count...";

	VkSampleCountFlagBits requested = samples;
	if (const auto fromEnvironment = msaaSamplesFromEnvironment()) {
		this->getLogger().info()
			<< "MSAA sample count forced to " << *fromEnvironment << " by "
			<< kMsaaSamplesVariable << ".";
		requested = *fromEnvironment;
	}

	if ((requested & kSupportedSampleCountMask) == 0) {
		this->getLogger().warning()
			<< "Requested MSAA sample count " << requested
			<< " is above the supported cap "
			<< this->getMaxSupportedMsaaSamples() << ", keeping "
			<< _msaaSamples;
		return;
	}

	const VkSampleCountFlags counts = this->getFramebufferSampleCounts();
	if ((counts & requested) == 0) {
		this->getLogger().warning()
			<< "Requested MSAA sample count " << requested
			<< " is not supported by color and depth attachments, keeping "
			<< _msaaSamples;
		return;
	}

	_msaaSamples = requested;
	this->getLogger().info() << "MSAA sample count set to " << requested;
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

evan::TransferManager &evan::DeviceContext::getTransferManager()
{
	return _transferManager;
}

const evan::PipelineCache &evan::DeviceContext::getPipelineCache() const
{
	return _pipelineCache;
}

VkSampleCountFlags evan::DeviceContext::getFramebufferSampleCounts() const
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(_deviceBackend->getPhysicalDevice(),
								  &physicalDeviceProperties);

	const VkSampleCountFlags counts =
		physicalDeviceProperties.limits.framebufferColorSampleCounts
		& physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	this->getLogger().info()
		<< "Sample counts supported for color and depth: " << counts;
	return counts;
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
		vkCreateCommandPool(_deviceBackend->getDevice(), &commandPoolCreateInfo,
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
	vkGetDeviceQueue(_deviceBackend->getDevice(),
					 indices.graphicsFamily.value(), 0, &_graphicsQueue);
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

bool evan::DeviceContext::checkDebugUtilsEnabled(VkInstance instance)
{
	this->getLogger().info()
		<< "Checking if VK_EXT_debug_utils is enabled on the instance...";

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");

	if (func == nullptr) {
		this->getLogger().warning()
			<< "VK_EXT_debug_utils is not enabled on the instance.";
		return false;
	}
	this->getLogger().info()
		<< "VK_EXT_debug_utils is enabled on the instance.";
	return true;
}

void evan::DeviceContext::setupDebugMessenger()
{
	this->getLogger().info() << "Setting up Vulkan debug messenger...";
	if (!enableValidationLayers) {
		this->getLogger().warning() << "Validation layers are disabled, "
									   "skipping debug messenger setup.";
		return;
	}

	if (!checkDebugUtilsSupport(_deviceBackend->getInstance())) {
		this->getLogger().warning() << "Debug utils extension not supported, "
									   "skipping debug messenger setup.";
		return;
	}

	if (!checkDebugUtilsEnabled(_deviceBackend->getInstance())) {
		this->getLogger().warning() << "Debug utils extension not enabled on "
									   "the instance, skipping debug messenger "
									   "setup.";
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo;

	this->populateDebugMessengerCreateInfo(createInfo, defaultDebugCallback);
	if (this->createDebugUtilsMessengerEXT(_deviceBackend->getInstance(),
										   &createInfo, nullptr,
										   &_debugMessenger)
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

void evan::DeviceContext::destroyDebugUtilsMessengerEXT(
	VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger)
{
	this->getLogger().info() << "Destroying debug utils messenger...";

	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) {
		func(instance, debugMessenger, nullptr);
		this->getLogger().info()
			<< "Debug utils messenger destroyed successfully.";
	} else {
		this->getLogger().warning()
			<< "vkDestroyDebugUtilsMessengerEXT not found, cannot destroy "
			   "debug messenger.";
	}
}
