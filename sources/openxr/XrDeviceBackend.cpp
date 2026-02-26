/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrDeviceBackend
*/

#include "openxr/XrDeviceBackend.hpp"

evan::XrDeviceBackend::XrDeviceBackend()
{
}

evan::XrDeviceBackend::~XrDeviceBackend()
{
    xrDestroySession(_session);
    xrDestroyInstance(_XrInstance);
}

void evan::XrDeviceBackend::init(const IPlatform& platform)
{
    createXrInstance(platform);
    getSystem();
}

VkInstance evan::XrDeviceBackend::createInstance(const IPlatform& platform, const std::string &appName, Version &appVersion)
{
    VkInstance instance = VK_NULL_HANDLE;
    XrGraphicsRequirementsVulkan2KHR graphicsRequirements {};
    PFN_xrGetVulkanGraphicsRequirements2KHR getVulkanGraphicsRequirements2KHR = nullptr;
    PFN_xrCreateVulkanInstanceKHR createVulkanInstanceKHR = nullptr;
    std::vector<const char*> extensions;
    std::vector<const char*> layers;

    graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR;
    if (xrGetInstanceProcAddr(_XrInstance, "xrGetVulkanGraphicsRequirements2KHR", reinterpret_cast<PFN_xrVoidFunction*>(&getVulkanGraphicsRequirements2KHR)) != XR_SUCCESS) {
        std::cerr << "Failed to get xrGetVulkanGraphicsRequirements2KHR function pointer." << std::endl;
        return instance;
    }
    if (xrGetInstanceProcAddr(_XrInstance, "xrCreateVulkanInstanceKHR", reinterpret_cast<PFN_xrVoidFunction*>(&createVulkanInstanceKHR)) != XR_SUCCESS) {
        std::cerr << "Failed to get xrCreateVulkanInstanceKHR function pointer." << std::endl;
        return instance;
    }

    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = appVersion.to_uint32_t();
    appInfo.pEngineName = "evan";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    if (enableValidationLayers) {
        layers = this->getRequiredInstanceExtensions();
        if (!layers.empty()) {
            extensions = this->getRequiredInstanceExtensionsAndroid();
        }
    }

    VkInstanceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();

    XrVulkanInstanceCreateInfoKHR vulkanCreateInfoKHR {};
    vulkanCreateInfoKHR.type = XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR;
    vulkanCreateInfoKHR.vulkanCreateInfo = &createInfo;
    vulkanCreateInfoKHR.systemId = _systemId;
    vulkanCreateInfoKHR.pfnGetInstanceProcAddr = &vkGetInstanceProcAddr;

    if (createVulkanInstanceKHR(_XrInstance, &vulkanCreateInfoKHR, &instance) != XR_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance through OpenXR." << std::endl;
        return VK_NULL_HANDLE;
    }
    return instance;
}

VkDevice evan::XrDeviceBackend::createLogicalDevice(VkPhysicalDevice physicalDevice)
{
    if (physicalDevice == VK_NULL_HANDLE) {
        std::cerr << "Invalid Vulkan physical device provided." << std::endl;
        return VK_NULL_HANDLE;
    }

    VkDevice device = VK_NULL_HANDLE;
    PFN_xrCreateVulkanDeviceKHR createVulkanDeviceKHR = nullptr;

    if (xrGetInstanceProcAddr(_XrInstance, "xrCreateVulkanDeviceKHR", reinterpret_cast<PFN_xrVoidFunction*>(&createVulkanDeviceKHR)) != XR_SUCCESS) {
        std::cerr << "Failed to get xrCreateVulkanDeviceKHR function pointer." << std::endl;
        return device;
    }
    evan::QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    if (!indices.isComplete()) {
        std::cerr << "Failed to find required queue families for the Vulkan device." << std::endl;
        return VK_NULL_HANDLE;
    }

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures {};

    VkDeviceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;

    XrVulkanDeviceCreateInfoKHR vulkanDeviceCreateInfoKHR {};
    vulkanDeviceCreateInfoKHR.type = XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR;
    vulkanDeviceCreateInfoKHR.systemId = _systemId;
    vulkanDeviceCreateInfoKHR.pfnGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanDeviceCreateInfoKHR.vulkanCreateInfo = &createInfo;
    vulkanDeviceCreateInfoKHR.vulkanPhysicalDevice = physicalDevice;

    if (createVulkanDeviceKHR(_XrInstance, &vulkanDeviceCreateInfoKHR, &device) != XR_SUCCESS) {
        std::cerr << "Failed to create Vulkan device through OpenXR." << std::endl;
        return VK_NULL_HANDLE;
    }
    return device;
}

VkPhysicalDevice evan::XrDeviceBackend::pickPhysicalDevice(VkInstance instance)
{
    if (instance == VK_NULL_HANDLE) {
        std::cerr << "Invalid Vulkan instance provided." << std::endl;
        return VK_NULL_HANDLE;
    }
    PFN_xrGetVulkanGraphicsDevice2KHR getVulkanGraphicsDevice2KHR = nullptr;
    XrVulkanGraphicsDeviceGetInfoKHR deviceGetInfo {};
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    if (xrGetInstanceProcAddr(_XrInstance, "xrGetVulkanGraphicsDevice2KHR", reinterpret_cast<PFN_xrVoidFunction*>(&getVulkanGraphicsDevice2KHR)) != XR_SUCCESS) {
        std::cerr << "Failed to get xrGetVulkanGraphicsDevice2KHR function pointer." << std::endl;
        return VK_NULL_HANDLE;
    }
    deviceGetInfo.type = XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR;
    deviceGetInfo.systemId = _systemId;
    deviceGetInfo.vulkanInstance = instance;

    if (getVulkanGraphicsDevice2KHR(_XrInstance, &deviceGetInfo, &physicalDevice) != XR_SUCCESS) {
        std::cerr << "Failed to get Vulkan physical device through OpenXR." << std::endl;
        return VK_NULL_HANDLE;
    }
    return physicalDevice;

}

void evan::XrDeviceBackend::createXrInstance(const IPlatform& platform)
{
    std::vector<const char*> extensions;

    for (const auto &ext : platform.getRequiredInstanceExtensions()) {
        extensions.push_back(ext.c_str());
    }

    XrInstanceCreateInfo createInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.enabledExtensionNames = extensions.data();
    // createInfo.enabledApiLayerNames = nullptr;
	// std::strcpy(createInfo.applicationInfo.applicationName, "evan");
	// createInfo.applicationInfo.apiVersion = #CURRENT_OPENXR_VERSION;

    createInfo.next = dynamic_cast<evan::XrPlatform*>(const_cast<IPlatform*>(&platform))->getInstanceCreateInfoAndroid();

    XrResult result = xrCreateInstance(&createInfo, &_XrInstance);
    if (result != XR_SUCCESS) {
        // TODO: Throw an exception or handle the error appropriately
        std::cerr << "Failed to create OpenXR instance: " << result << std::endl;
        return;
    }
}

void evan::XrDeviceBackend::getSystem()
{
    XrSystemGetInfo systemInfo{ XR_TYPE_SYSTEM_GET_INFO };
    systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    XrResult result = xrGetSystem(_XrInstance, &systemInfo, &_systemId);
    if (result != XR_SUCCESS) {
        // TODO: Throw an exception or handle the error appropriately
        std::cerr << "Failed to get OpenXR system: " << result << std::endl;
        return;
    }
}

void evan::XrDeviceBackend::createSession(VkDevice device)
{

}


std::vector<VkExtensionProperties> evan::XrDeviceBackend::getInstanceExtensions() const
{
    uint32_t extensionCount = 0;
    std::vector<VkExtensionProperties> extensions;
    std::string layerName = "VK_LAYER_KHRONOS_validation";

    if (vkEnumerateInstanceExtensionProperties(layerName.c_str(), &extensionCount, nullptr) != XR_SUCCESS) {
        std::cerr << "Failed to enumerate OpenXR instance extension properties." << std::endl;
        return extensions;
    }

    extensions.resize(extensionCount);
    if (vkEnumerateInstanceExtensionProperties(layerName.c_str(), &extensionCount, extensions.data()) != XR_SUCCESS) {
        std::cerr << "Failed to enumerate OpenXR instance extension properties." << std::endl;
        return {};
    }

    return extensions;
}

std::vector<const char*> evan::XrDeviceBackend::getRequiredInstanceExtensions() const
{
    auto availableExtensions = getInstanceExtensions();
    std::vector<const char*> requiredExtensions;

    for (const auto &ext : availableExtensions) {
        if (std::strcmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        } else if (std::strcmp(ext.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0) {
            requiredExtensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
        }
    }
    return requiredExtensions;
}

std::vector<const char*> evan::XrDeviceBackend::getRequiredInstanceExtensionsAndroid()
{
    std::vector<const char*> availableLayers;

    for (const auto &layer : this->getAvailableLayers()) {
        if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
            availableLayers.emplace_back(layer.layerName);
        }
    }
    return availableLayers;
}