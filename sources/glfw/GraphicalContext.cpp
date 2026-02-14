/*
** EPITECH PROJECT, 2025
** evan
** File description:
** GraphicalContext
*/

#include "glfw/GraphicalContext.hpp"

//////////////////////
// Static functions //
//////////////////////

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

////////////////////
// Public methods //
////////////////////

evan::glfw::GraphicalContext::GraphicalContext()
{
	_appName	   = "Hello, World !";
	_appVersion	   = new Version(1, 0, 0);
	_engineName	   = "Evan";
	_engineVersion = new Version(1, 0, 0);

	this->initWindow(800, 600);

	this->createInstance();

	_renderingContext =
		std::make_shared<evan::glfw::RenderingContext>(_instance, _window);

	auto vulkanContext = _renderingContext->getVulkanContext();
	evan::glfw::SwapchainContext::SwapchainContextCreationProperties
		swapchainProperties = { ._surface = vulkanContext->surface,
								._physicalDevice =
									vulkanContext->physicalDevice,
								._logicalDevice = vulkanContext->logicalDevice,
								._window		= _window,
								._msaaSamples	= vulkanContext->msaaSamples,
								._commandPool	= vulkanContext->commandPool,
								._graphicsQueue = vulkanContext->graphicsQueue,
								._instance		= _instance };

	_swapchainContext =
		std::make_shared<evan::glfw::SwapchainContext>(swapchainProperties);
}

evan::glfw::GraphicalContext::GraphicalContext(const std::string &appName,
											   const Version &appVersion,
											   const std::string &engineName,
											   const Version &engineVersion,
											   unsigned int windowWidth,
											   unsigned int windowHeight)
{
	_appName	   = appName;
	_appVersion	   = new Version(appVersion);
	_engineName	   = engineName;
	_engineVersion = new Version(engineVersion);

	this->initWindow(windowWidth, windowHeight);

	this->createInstance();

	_renderingContext =
		std::make_shared<evan::glfw::RenderingContext>(_instance, _window);

	auto vulkanContext = _renderingContext->getVulkanContext();
	evan::glfw::SwapchainContext::SwapchainContextCreationProperties
		swapchainProperties = { ._surface = vulkanContext->surface,
								._physicalDevice =
									vulkanContext->physicalDevice,
								._logicalDevice = vulkanContext->logicalDevice,
								._window		= _window,
								._msaaSamples	= vulkanContext->msaaSamples,
								._commandPool	= vulkanContext->commandPool,
								._graphicsQueue = vulkanContext->graphicsQueue,
								._instance		= _instance };

	_swapchainContext =
		std::make_shared<evan::glfw::SwapchainContext>(swapchainProperties);
}

evan::glfw::GraphicalContext::~GraphicalContext()
{
	delete _appVersion;
	delete _engineVersion;

	if (enableValidationLayers) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			_instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(_instance, nullptr, nullptr);
		}
	}
	vkDestroyInstance(_instance, nullptr);
}

void evan::glfw::GraphicalContext::createInstance()
{
	if (enableValidationLayers && !this->checkValidationLayerSupport()) {
		throw std::runtime_error(
			"Validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo {};
	appInfo.sType			   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = _appName.c_str();
	appInfo.applicationVersion = _appVersion->to_uint32_t();
	appInfo.pEngineName		   = _engineName.c_str();
	appInfo.engineVersion	   = _engineVersion->to_uint32_t();
	appInfo.apiVersion		   = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo {};
	createInfo.sType			= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = this->getInstanceExtensions();
	// Because the extensions are stored as std::string, we need to convert them
	// to const char* to pass them to the Vulkan API.
	auto extensionsWrapped = std::vector<const char *>(extensions.size());

	std::transform(extensions.begin(), extensions.end(),
				   extensionsWrapped.begin(), [](const std::string &str) {
					   return str.c_str();
				   });
	createInfo.enabledExtensionCount =
		static_cast<uint32_t>(extensionsWrapped.size());
	createInfo.ppEnabledExtensionNames = extensionsWrapped.data();

	createInfo.enabledLayerCount =
		static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();

	if (enableValidationLayers == true) {
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};

		this->populateDebugMessengerCreateInfo(debugCreateInfo,
											   defaultDebugCallback);
		createInfo.pNext =
			(VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);

	if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
		std::cout << "Failed to create instance due to incompatible driver ! "
					 "Trying with MacOS settings..."
				  << std::endl;

		createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

		createInfo.enabledExtensionCount   = (uint32_t)extensionsWrapped.size();
		createInfo.ppEnabledExtensionNames = extensionsWrapped.data();

		result = vkCreateInstance(&createInfo, nullptr, &_instance);
		switch (result) {
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				throw std::runtime_error("Host out of memory !");
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				throw std::runtime_error("Device out of memory !");
			case VK_ERROR_INITIALIZATION_FAILED:
				throw std::runtime_error("Initialization failed !");
			case VK_ERROR_LAYER_NOT_PRESENT:
				throw std::runtime_error("Layer not present !");
			case VK_ERROR_EXTENSION_NOT_PRESENT:
				throw std::runtime_error("Extension not present !");
			case VK_ERROR_INCOMPATIBLE_DRIVER:
				throw std::runtime_error("Incompatible driver !");
			default:
				break;
		}
	}
	if (result == VK_SUCCESS) {
		std::cout << "Instance created !" << std::endl;
	} else {
		throw std::runtime_error("Failed to create instance !");
	}
}

void evan::glfw::GraphicalContext::initWindow(int width, int height)
{
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	_window =
		glfwCreateWindow(width, height, _appName.c_str(), nullptr, nullptr);
	if (!_window) {
		throw std::runtime_error("Failed to create GLFW window");
	}
}

std::vector<std::string> evan::glfw::GraphicalContext::getInstanceExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions =
		glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char *> extensions(glfwExtensions,
										 glfwExtensions + glfwExtensionCount);

	extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	if (enableValidationLayers == true) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return std::vector<std::string>(extensions.begin(), extensions.end());
}

/////////////////////
// Private methods //
/////////////////////

void evan::glfw::GraphicalContext::populateDebugMessengerCreateInfo(
	VkDebugUtilsMessengerCreateInfoEXT &createInfo,
	debugCallback_t debugCallback)
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

bool evan::glfw::GraphicalContext::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char *layerName: validationLayers) {
		bool layerFound = false;

		for (const auto &layerProperties: availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}
	return true;
}
