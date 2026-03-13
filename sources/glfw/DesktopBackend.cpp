/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopBackend
*/

#include "glfw/DesktopBackend.hpp"

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

evan::DesktopBackend::DesktopBackend(const DesktopPlatform &platform)
{
	Version appVersion(0, 1, 0);

	this->createInstance(platform, "Evan", appVersion);
	this->createSurface(_VkInstance, platform._window);
	this->pickPhysicalDevice();
	this->createLogicalDevice();
}

evan::DesktopBackend::~DesktopBackend()
{
}

VkSurfaceKHR evan::DesktopBackend::createSurface(VkInstance instance,
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

	return _surface;
}

void evan::DesktopBackend::createPresentQueue()
{
	QueueFamilyIndices indices = this->findQueueFamilies();

	vkGetDeviceQueue(_device, indices.presentFamily.value(), 0,
					 &_presentQueue);
}

void evan::DesktopBackend::createInstance(const evan::IPlatform &platform,
										  const std::string &appName,
										  evan::Version &appVersion)
{
	if (enableValidationLayers && !this->checkValidationLayerSupport()) {
		throw std::runtime_error(
			"Validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo {};
	appInfo.sType			   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = appName.c_str();
	appInfo.applicationVersion = appVersion.to_uint32_t();
	appInfo.pEngineName		   = "Evan";
	appInfo.engineVersion	   = VK_MAKE_VERSION(0, 1, 0);
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

	VkResult result = vkCreateInstance(&createInfo, nullptr, &_VkInstance);

	if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
		std::cout << "Failed to create instance due to incompatible driver ! "
					 "Trying with MacOS settings..."
				  << std::endl;

		createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

		createInfo.enabledExtensionCount   = (uint32_t)extensionsWrapped.size();
		createInfo.ppEnabledExtensionNames = extensionsWrapped.data();

		result = vkCreateInstance(&createInfo, nullptr, &_VkInstance);
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

evan::SwapChainSupportDetails
	evan::DesktopBackend::querySwapChainSupport(VkPhysicalDevice device,
												VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;
	uint32_t formatCount;
	uint32_t presentModeCount;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
											  &details.capabilities);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
											 nullptr)
		!= VK_SUCCESS)
		return details;
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
											 details.formats.data());
	}

	if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
												  &presentModeCount, nullptr)
		!= VK_SUCCESS)
		return details;
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}


bool evan::DesktopBackend::isDeviceSuitable(
	VkPhysicalDevice device, VkSurfaceKHR surface,
	std::vector<const char *> deviceExtensions)
{
	// Temporary assignment to check device properties and features before picking the device.
	// This will be reset to VK_NULL_HANDLE if the device is not suitable.
	_physicalDevice = device;

	QueueFamilyIndices indices = this->findQueueFamilies();

	_physicalDevice = VK_NULL_HANDLE;
	bool extensionsSupported =
		this->checkDeviceExtensionSupport(device, deviceExtensions);
	bool swapChainAdequate = false;

	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport =
			this->querySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty()
			&& !swapChainSupport.presentModes.empty();
	}
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate
		&& supportedFeatures.samplerAnisotropy;
}

bool evan::DesktopBackend::checkDeviceExtensionSupport(
	VkPhysicalDevice device, std::vector<const char *> deviceExtensions)
{
	uint32_t extensionCount;
	if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
											 nullptr)
		!= VK_SUCCESS)
		return false;

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
										 availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(),
											 deviceExtensions.end());

	for (const auto &extension: availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool evan::DesktopBackend::checkValidationLayerSupport()
{
	auto availableLayers = this->getAvailableLayers();

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

std::vector<std::string> evan::DesktopBackend::getInstanceExtensions()
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

void evan::DesktopBackend::populateDebugMessengerCreateInfo(
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

void evan::DesktopBackend::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(_VkInstance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(_VkInstance, &deviceCount, devices.data());

	for (const auto &device: devices) {
		if (this->isDeviceSuitable(device, _surface, deviceExtensions)) {
			_physicalDevice = device;
			break;
		}
	}

	if (_physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("Failed to find a suitable GPU!");
	}
}

void evan::DesktopBackend::createLogicalDevice()
{
	QueueFamilyIndices indices = this->findQueueFamilies();

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

	if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}
}

evan::QueueFamilyIndices evan::DesktopBackend::findQueueFamilies()
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount,
											 nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount,
											 queueFamilies.data());

	for (uint32_t i = 0; i < queueFamilies.size(); i++) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface,
											 &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}
	}

	return indices;
}

uint32_t evan::DesktopBackend::countSwapchainFormats() const
{
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount,
										 nullptr);
	return formatCount;
}

std::vector<int64_t> evan::DesktopBackend::enumerateSwapchainFormats(uint32_t swapchainFormatCount) const
{
	std::vector<int32_t> swapchainFormats(swapchainFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &swapchainFormatCount,
										 reinterpret_cast<VkSurfaceFormatKHR *>(swapchainFormats.data()));

	std::vector<int64_t> swapchainFormats64(swapchainFormatCount);
	std::transform(swapchainFormats.begin(), swapchainFormats.end(),
				   swapchainFormats64.begin(), [](int32_t format) {
					   return static_cast<int64_t>(format);
				   });
	return swapchainFormats64;
}
