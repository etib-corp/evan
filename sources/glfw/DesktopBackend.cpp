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

}

evan::DesktopBackend::~DesktopBackend()
{
}

VkSurfaceKHR evan::DesktopBackend::createSurface(VkInstance instance, GLFWwindow *window)
{
    VkSurfaceKHR surface;

#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR createInfo {};

	createInfo.sType	 = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd		 = glfwGetWin32Window(window);
	createInfo.hinstance = GetModuleHandle(nullptr);

	if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}
#else
	if (instance == VK_NULL_HANDLE) {
		throw std::runtime_error("Vulkan instance is NULL!");
	}
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}
#endif

    return surface;
}

VkInstance evan::DesktopBackend::createInstance(const evan::IPlatform &platform, const std::string &appName, evan::Version &appVersion)
{
    VkInstance instance;

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

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

	if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
		std::cout << "Failed to create instance due to incompatible driver ! "
					 "Trying with MacOS settings..."
				  << std::endl;

		createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

		createInfo.enabledExtensionCount   = (uint32_t)extensionsWrapped.size();
		createInfo.ppEnabledExtensionNames = extensionsWrapped.data();

		result = vkCreateInstance(&createInfo, nullptr, &instance);
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
	return instance;
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
