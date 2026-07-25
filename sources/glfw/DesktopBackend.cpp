/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopBackend
*/

#include "evan/glfw/DesktopBackend.hpp"

/**
 * @brief Default debug callback function for Vulkan validation layers.
 *
 * This function is called by the Vulkan validation layers when a validation
 * message is generated. It receives the severity and type of the message, as
 * well as the callback data containing details about the message. The function
 * simply prints the validation message to the standard error stream and returns
 * VK_FALSE to indicate that the application should not be aborted.
 *
 * @param messageSeverity The severity of the validation message (e.g., error,
 * warning, info).
 * @param messageType The type of the validation message (e.g., general,
 * performance, validation).
 * @param pCallbackData A pointer to a structure containing details about the
 * validation message, such as the message string and any associated objects or
 * data.
 * @param pUserData A pointer to user-defined data that can be passed to the
 * callback function. This parameter is not used in this default implementation,
 * but it can be utilized to provide additional context or information when
 * handling validation messages.
 *
 * @return VK_FALSE to indicate that the application should not be aborted,
 * allowing the validation layers to continue processing and reporting messages
 * as needed.
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

evan::DesktopBackend::DesktopBackend(const IPlatform &platform)
{
	this->getLogger().info() << "Initializing DesktopBackend...";

	Version appVersion(0, 1, 0);

	this->getLogger().info()
		<< "Setting up GLFW platform for DesktopBackend...";
	auto glfwPlatform = dynamic_cast<const IDesktopPlatform *>(&platform);

	this->createInstance(platform, "Evan", appVersion);

	this->getLogger().info() << "Creating Vulkan surface for DesktopBackend...";
	_surface = glfwPlatform->createSurface(_VkInstance);

	this->pickPhysicalDevice();
	this->createLogicalDevice();
	this->createPresentQueue();

	this->setupCallbackEvent(*glfwPlatform);
}

evan::DesktopBackend::~DesktopBackend()
{
	this->getLogger().info()
		<< "Destroying DesktopBackend and cleaning up Vulkan resources...";
	vkDestroyDevice(_device, nullptr);

	this->getLogger().info() << "Destroying Vulkan surface...";
	vkDestroySurfaceKHR(_VkInstance, _surface, nullptr);

	this->getLogger().info() << "Destroying Vulkan instance...";
	vkDestroyInstance(_VkInstance, nullptr);
}

////////////////////
// Public Methods //
////////////////////

bool evan::DesktopBackend::processFrame(VkPresentInfoKHR presentInfo,
										ASwapchainImage &swapchainImage)
{
	this->getLogger().info() << "Processing frame for DesktopBackend...";
	VkResult result = vkQueuePresentKHR(_presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		this->getLogger().info() << "Swap chain is out of date or suboptimal.";
		return false;
	} else if (result != VK_SUCCESS) {
		this->getLogger().error() << "Failed to present swap chain image!";
		return false;
	}
	this->getLogger().info() << "Frame presented successfully.";
	return true;
}

uint32_t evan::DesktopBackend::countSwapchainFormats() const
{
	this->getLogger().info()
		<< "Counting available swap chain formats for DesktopBackend...";

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface,
										 &formatCount, nullptr);

	this->getLogger().info()
		<< "Found " << formatCount << " swap chain formats available.";
	return formatCount;
}

std::vector<int64_t> evan::DesktopBackend::enumerateSwapchainFormats(
	uint32_t swapchainFormatCount) const
{
	this->getLogger().info()
		<< "Enumerating swap chain formats for DesktopBackend...";

	std::vector<VkSurfaceFormatKHR> swapchainFormats(swapchainFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface,
										 &swapchainFormatCount,
										 swapchainFormats.data());

	std::vector<int64_t> swapchainFormats64(swapchainFormatCount);
	for (size_t i = 0; i < swapchainFormatCount; i++) {
		swapchainFormats64[i] =
			static_cast<int64_t>(swapchainFormats[i].format);
	}
	this->getLogger().info() << "Enumerated " << swapchainFormatCount
							 << " swap chain formats successfully.";
	return swapchainFormats64;
}

evan::QueueFamilyIndices evan::DesktopBackend::findQueueFamilies()
{
	this->getLogger().info() << "Finding queue families for DesktopBackend...";

	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount,
											 nullptr);

	this->getLogger().info()
		<< "Found " << queueFamilyCount << " queue families available.";

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount,
											 queueFamilies.data());

	this->getLogger().info()
		<< "Checking queue families for graphics and presentation support...";
	for (uint32_t i = 0; i < queueFamilies.size(); i++) {
		this->getLogger().info() << "Checking queue family " << i << " with "
								 << queueFamilies[i].queueCount << " queues...";
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;

		this->getLogger().info()
			<< "Checking if queue family " << i << " supports presentation...";
		vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface,
											 &presentSupport);

		if (presentSupport) {
			this->getLogger().info()
				<< "Queue family " << i << " supports presentation.";
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			this->getLogger().info() << "Found suitable queue families for "
										"graphics and presentation support.";
			break;
		}
	}

	this->getLogger().info()
		<< "Queue family indices found: Graphics Family = "
		<< (indices.graphicsFamily.has_value()
				? std::to_string(indices.graphicsFamily.value())
				: "None")
		<< ", Present Family = "
		<< (indices.presentFamily.has_value()
				? std::to_string(indices.presentFamily.value())
				: "None");
	return indices;
}

evan::SwapChainSupportDetails evan::DesktopBackend::querySwapChainSupport()
{
	this->getLogger().info()
		<< "Querying swap chain support details for DesktopBackend...";

	SwapChainSupportDetails details;
	uint32_t formatCount;
	uint32_t presentModeCount;

	this->getLogger().info() << "Querying surface capabilities...";
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface,
											  &details.capabilities);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface,
											 &formatCount, nullptr)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to query surface formats!";
		this->getLogger().info()
			<< "Returning swap chain support details with capabilities but no "
			   "formats or present modes.";
		return details;
	}
	if (formatCount != 0) {
		this->getLogger().info()
			<< "Found " << formatCount << " surface formats available.";
		details.formats.resize(formatCount);

		this->getLogger().info() << "Querying surface formats...";
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			_physicalDevice, _surface, &formatCount, details.formats.data());
	}

	this->getLogger().info() << "Querying surface present modes...";
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface,
												  &presentModeCount, nullptr)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to query surface present modes!";
		this->getLogger().info()
			<< "Returning swap chain support details with capabilities and "
			   "formats but no present modes.";
		return details;
	}
	if (presentModeCount != 0) {
		this->getLogger().info() << "Found " << presentModeCount
								 << " surface present modes available.";
		details.presentModes.resize(presentModeCount);
		this->getLogger().info() << "Querying surface present modes...";
		vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface,
												  &presentModeCount,
												  details.presentModes.data());
	}
	this->getLogger().info()
		<< "Swap chain support details queried successfully.";
	return details;
}

void evan::DesktopBackend::createPresentQueue()
{
	this->getLogger().info() << "Creating present queue for DesktopBackend...";

	QueueFamilyIndices indices = this->findQueueFamilies();

	this->getLogger().info()
		<< "Retrieving present queue from logical device...";

	vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);
}

///////////////////////
// Protected methods //
///////////////////////

void evan::DesktopBackend::createInstance(const evan::IPlatform &platform,
										  const std::string &appName,
										  evan::Version &appVersion)
{
	this->getLogger().info()
		<< "Creating Vulkan instance for DesktopBackend...";

	bool useValidationLayers = enableValidationLayers;
	if (useValidationLayers && !this->checkValidationLayerSupport()) {
		this->getLogger().warning()
			<< "Validation layers requested, but not available. Continuing "
			   "without them.";
		useValidationLayers = false;
	}

	VkApplicationInfo appInfo {};
	appInfo.sType			   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = appName.c_str();
	appInfo.applicationVersion = appVersion.to_uint32_t();
	appInfo.pEngineName		   = "Evan";
	appInfo.engineVersion	   = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion		   = VK_API_VERSION_1_1;

	this->getLogger().info()
		<< "Setting up instance creation information using application info:\n"
		   "Application Name = "
		<< appName << ",\nApplication Version = " << appVersion._major << "."
		<< appVersion._minor << "." << appVersion._patch
		<< ",\nEngine Name = Evan,\nEngine Version = 0.1.0,\nAPI Version = 1.1";

	VkInstanceCreateInfo createInfo {};
	createInfo.sType			= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = platform.getRequiredInstanceExtensions();

	this->getLogger().info()
		<< "Required instance extensions for platform: " << extensions.size()
		<< " extensions found.";
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

	if (useValidationLayers) {
		createInfo.enabledLayerCount =
			static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		this->getLogger().info()
			<< "Validation layers enabled for instance creation. Setting up "
			   "debug messenger create info...";
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};

		this->populateDebugMessengerCreateInfo(debugCreateInfo,
											   defaultDebugCallback);
		createInfo.pNext =
			(VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	this->getLogger().info()
		<< "Creating Vulkan instance with the specified application info, "
		   "extensions, and validation layers...";
	VkResult result = vkCreateInstance(&createInfo, nullptr, &_VkInstance);

	if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
		this->getLogger().warning()
			<< "Failed to create Vulkan instance due to incompatible driver. "
			   "This may be caused by running on a platform that requires the "
			   "VK_KHR_portability_subset extension, such as MacOS. Retrying "
			   "instance creation with VK_KHR_portability_subset enabled...";

		createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

		createInfo.enabledExtensionCount   = (uint32_t)extensionsWrapped.size();
		createInfo.ppEnabledExtensionNames = extensionsWrapped.data();

		result = vkCreateInstance(&createInfo, nullptr, &_VkInstance);
		switch (result) {
			case VK_SUCCESS:
				this->getLogger().info()
					<< "Vulkan instance created successfully with "
					   "VK_KHR_portability_subset enabled!";
				break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				this->getLogger().error()
					<< "Host out of memory while creating Vulkan instance!";
				return;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				this->getLogger().error()
					<< "Device out of memory while creating Vulkan instance!";
				return;
			case VK_ERROR_INITIALIZATION_FAILED:
				this->getLogger().error()
					<< "Vulkan initialization failed while creating instance!";
				return;
			case VK_ERROR_LAYER_NOT_PRESENT:
				this->getLogger().error() << "Validation layer not present "
											 "while creating Vulkan instance!";
				return;
			case VK_ERROR_EXTENSION_NOT_PRESENT:
				this->getLogger().error() << "Required extension not present "
											 "while creating Vulkan instance!";
				return;
			case VK_ERROR_INCOMPATIBLE_DRIVER:
				this->getLogger().error()
					<< "Failed to create Vulkan instance due to incompatible "
					   "driver, even with VK_KHR_portability_subset enabled! "
					   "This may indicate a deeper issue with the Vulkan "
					   "implementation on this platform.";
				return;
			default:
				this->getLogger().error() << "Failed to create Vulkan instance "
											 "due to unknown error! VkResult: "
										  << result;
				return;
		}
	}
	if (result == VK_SUCCESS) {
		this->getLogger().info() << "Vulkan instance created successfully!";
	} else {
		this->getLogger().error()
			<< "Failed to create Vulkan instance! VkResult: " << result;
	}
}

void evan::DesktopBackend::createLogicalDevice()
{
	this->getLogger().info() << "Creating logical device for DesktopBackend...";

	std::vector<const char *> desktopExtensions = deviceExtensions;

#ifdef __APPLE__
	desktopExtensions.push_back("VK_KHR_portability_subset");
#endif

	this->getLogger().info()
		<< "Required device extensions for logical device creation: "
		<< desktopExtensions.size() << " extensions required.";
	std::string extensionsList;
	for (const char *ext: desktopExtensions) {
		extensionsList += std::string(ext) + ", ";
	}
	this->getLogger().info() << "Device extensions: " << extensionsList;

	QueueFamilyIndices indices = this->findQueueFamilies();

	this->getLogger().info()
		<< "Setting up queue create infos for logical device creation...";

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),
											   indices.presentFamily.value() };

	float queuePriority = 1.0f;

	this->getLogger().info()
		<< "Unique queue families identified: " << uniqueQueueFamilies.size()
		<< " unique queue families found.";
	for (uint32_t queueFamily: uniqueQueueFamilies) {
		this->getLogger().info()
			<< "Setting up queue create info for queue family index "
			<< queueFamily << " with priority " << queuePriority << "...";
		VkDeviceQueueCreateInfo queueCreateInfo {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount		 = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures {};
	VkPhysicalDeviceFeatures supportedFeatures;

	this->getLogger().info()
		<< "Querying supported features of the physical device...";
	vkGetPhysicalDeviceFeatures(_physicalDevice, &supportedFeatures);

	deviceFeatures.samplerAnisotropy = supportedFeatures.samplerAnisotropy;
	deviceFeatures.sampleRateShading = supportedFeatures.sampleRateShading;

#ifdef DEV
	deviceFeatures.fillModeNonSolid = VK_TRUE;
#endif

	VkDeviceCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount =
		static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount =
		static_cast<uint32_t>(desktopExtensions.size());
	createInfo.ppEnabledExtensionNames = desktopExtensions.data();

	this->getLogger().info()
		<< "Enabling validation layers for logical device creation...";
	if (enableValidationLayers) {
		this->getLogger().info()
			<< "Validation layers enabled. Setting up validation layers for "
			   "logical device creation...";
		createInfo.enabledLayerCount =
			static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	} else {
		this->getLogger().info()
			<< "Validation layers not enabled. Skipping validation layer setup "
			   "for logical device creation.";
		createInfo.enabledLayerCount = 0;
	}

	this->getLogger().info()
		<< "Creating logical device with the specified queue create infos, "
		   "enabled features, extensions, and validation layers...";
	if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to create logical device!";
		return;
	}
	this->getLogger().info() << "Logical device created successfully!";
}

void evan::DesktopBackend::pickPhysicalDevice()
{
	this->getLogger().info() << "Picking physical device for DesktopBackend...";

	uint32_t deviceCount = 0;

	this->getLogger().info() << "Enumerating physical devices...";
	vkEnumeratePhysicalDevices(_VkInstance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		this->getLogger().error() << "Failed to find GPUs with Vulkan support!";
		return;
	}

	this->getLogger().info()
		<< "Found " << deviceCount
		<< " physical devices with Vulkan support. Evaluating suitability...";

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(_VkInstance, &deviceCount, devices.data());

	this->getLogger().info()
		<< "Checking each physical device for suitability...";
	for (const auto &device: devices) {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		this->getLogger().info() << "Evaluating physical device: "
								 << std::string(deviceProperties.deviceName);

		if (this->isDeviceSuitable(device, deviceExtensions)) {
			this->getLogger().info() << "Physical device "
									 << std::string(deviceProperties.deviceName)
									 << " is suitable. Selecting this device.";
			_physicalDevice = device;
			break;
		}
	}

	if (_physicalDevice == VK_NULL_HANDLE) {
		this->getLogger().error() << "Failed to find a suitable GPU!";
		return;
	}
	this->getLogger().info() << "Physical device selected successfully.";
}

/////////////////////
// Private Methods //
/////////////////////

bool evan::DesktopBackend::isDeviceSuitable(
	VkPhysicalDevice device, std::vector<const char *> deviceExtensions)
{
	_physicalDevice			   = device;
	QueueFamilyIndices indices = this->findQueueFamilies();

	bool extensionsSupported =
		this->checkDeviceExtensionSupport(device, deviceExtensions);
	bool swapChainAdequate = false;

	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport =
			this->querySwapChainSupport();
		swapChainAdequate = !swapChainSupport.formats.empty()
			&& !swapChainSupport.presentModes.empty();
	}
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	_physicalDevice = VK_NULL_HANDLE;

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

void evan::DesktopBackend::setupCallbackEvent(const IPlatform &platform)
{
	this->getLogger().info()
		<< "Setting up GLFW input callbacks for DesktopBackend...";

	auto glfwPlatform = dynamic_cast<const IDesktopPlatform *>(&platform);

	this->getLogger().info() << "Setting GLFW window user pointer to the "
								"platform instance for callback access...";
	glfwSetWindowUserPointer(glfwPlatform->_window, (void *)glfwPlatform);

	this->getLogger().info()
		<< "Setting GLFW key callback for keyboard input events...";
	glfwSetKeyCallback(
		glfwPlatform->_window,
		[](GLFWwindow *window, int key, int scancode, int action, int mods) {
			auto *self = static_cast<evan::IDesktopPlatform *>(
				glfwGetWindowUserPointer(window));

			(void)window;
			(void)scancode;
			auto event = std::make_shared<utility::event::KeyboardEvent>();
			event->setKeycode(self->convertGlfwKeyToKeyCode(key));
			event->setIsDownEvent(action == GLFW_PRESS
								  || action == GLFW_REPEAT);
			event->setModifiers(self->convertGlfwModsToKeyModifiers(mods));
			self->_keyboardEvents.push_back(std::move(event));
		});

	this->getLogger().info()
		<< "Setting GLFW mouse button callback for mouse input events...";
	glfwSetMouseButtonCallback(
		glfwPlatform->_window,
		[](GLFWwindow *window, int button, int action, int mods) {
			auto *self = static_cast<evan::IDesktopPlatform *>(
				glfwGetWindowUserPointer(window));

			(void)window;
			(void)mods;
			auto event = std::make_shared<utility::event::MouseButtonEvent>();
			event->setButton(self->convertGlfwMouseButtonToButton(button));
			event->setPressed(action == GLFW_PRESS);
			event->setPosition(self->getMousePosition());
			self->_mouseButtonEvents.push_back(std::move(event));
		});

	this->getLogger().info()
		<< "Setting GLFW cursor position callback for mouse motion events...";
	glfwSetCursorPosCallback(
		glfwPlatform->_window,
		[](GLFWwindow *window, double xpos, double ypos) {
			auto *self = static_cast<evan::IDesktopPlatform *>(
				glfwGetWindowUserPointer(window));

			auto event = std::make_shared<utility::event::MouseMotionEvent>();
			event->setPosition(utility::event::MouseMotionEvent::MousePosition {
				static_cast<float>(xpos), static_cast<float>(ypos) });
			self->_mouseMotionEvents.push_back(std::move(event));
		});

	this->getLogger().info()
		<< "Setting GLFW scroll callback for mouse wheel events...";
	glfwSetScrollCallback(
		glfwPlatform->_window,
		[](GLFWwindow *window, double xoffset, double yoffset) {
			auto *self = static_cast<evan::IDesktopPlatform *>(
				glfwGetWindowUserPointer(window));

			auto event = std::make_shared<utility::event::MouseWheelEvent>();
			event->setOffset(utility::math::Vector2F {
				static_cast<float>(xoffset), static_cast<float>(yoffset) });
			self->_mouseWheelEvents.push_back(std::move(event));
		});

	this->getLogger().info() << "Setting GLFW cursor enter callback for cursor "
								"enter/leave events...";
	glfwSetCursorEnterCallback(
		glfwPlatform->_window, [](GLFWwindow *window, int entered) {
			auto *self = static_cast<evan::IDesktopPlatform *>(
				glfwGetWindowUserPointer(window));

			auto event = std::make_shared<utility::event::CursorEnterEvent>();
			event->setEntered(entered == GLFW_TRUE);
			self->_cursorEnterEvents.push_back(std::move(event));
		});

	this->getLogger().info()
		<< "Setting GLFW drop callback for file drop events...";
	glfwSetDropCallback(
		glfwPlatform->_window,
		[](GLFWwindow *window, int count, const char **paths) {
			auto *self = static_cast<evan::IDesktopPlatform *>(
				glfwGetWindowUserPointer(window));

			auto event = std::make_shared<utility::event::FileDropEvent>();
			std::vector<std::string> pathVector;
			pathVector.reserve(count);
			for (int i = 0; i < count; i++) {
				pathVector.emplace_back(paths[i]);
			}
			event->setPaths(pathVector);
			self->_fileDropEvents.push_back(std::move(event));
		});

	this->getLogger().info()
		<< "Setting GLFW character callback for text input events...";
	glfwSetCharCallback(
		glfwPlatform->_window, [](GLFWwindow *window, unsigned int codepoint) {
			auto *self = static_cast<evan::IDesktopPlatform *>(
				glfwGetWindowUserPointer(window));

			auto event = std::make_shared<utility::event::TextInputEvent>();
			// Convert UTF-32 code point to UTF-8 string
			std::string utf8Text;
			if (codepoint <= 0x7F) {
				// ASCII range
				utf8Text = static_cast<char>(codepoint);
			} else if (codepoint <= 0x7FF) {
				// 2-byte UTF-8
				utf8Text += static_cast<char>(0xC0 | (codepoint >> 6));
				utf8Text += static_cast<char>(0x80 | (codepoint & 0x3F));
			} else if (codepoint <= 0xFFFF) {
				// 3-byte UTF-8
				utf8Text += static_cast<char>(0xE0 | (codepoint >> 12));
				utf8Text += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
				utf8Text += static_cast<char>(0x80 | (codepoint & 0x3F));
			} else if (codepoint <= 0x10FFFF) {
				// 4-byte UTF-8
				utf8Text += static_cast<char>(0xF0 | (codepoint >> 18));
				utf8Text +=
					static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
				utf8Text += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
				utf8Text += static_cast<char>(0x80 | (codepoint & 0x3F));
			}
			event->setText(utf8Text);
			self->_textInputEvents.push_back(std::move(event));
		});
}