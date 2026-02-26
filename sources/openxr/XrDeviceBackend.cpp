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

VkInstance evan::XrDeviceBackend::createInstance(const IPlatform& platform)
{
    VkInstance instance = VK_NULL_HANDLE;
    XrGraphicsRequirementsVulkan2KHR graphicsRequirements {};
    PFN_xrGetVulkanGraphicsRequirements2KHR getVulkanGraphicsRequirements2KHR = nullptr;
    PFN_xrCreateVulkanInstanceKHR createVulkanInstance = nullptr;
    std::vector<const char*> extensions;
    std::vector<const char*> layers;

    graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR;
    if (xrGetInstanceProcAddr(_XrInstance, "xrGetVulkanGraphicsRequirements2KHR", reinterpret_cast<PFN_xrVoidFunction*>(&getVulkanGraphicsRequirements2KHR)) != XR_SUCCESS) {
        std::cerr << "Failed to get xrGetVulkanGraphicsRequirements2KHR function pointer." << std::endl;
        return instance;
    }
    if (xrGetInstanceProcAddr(_XrInstance, "xrCreateVulkanInstanceKHR", reinterpret_cast<PFN_xrVoidFunction*>(&createVulkanInstance)) != XR_SUCCESS) {
        std::cerr << "Failed to get xrCreateVulkanInstanceKHR function pointer." << std::endl;
        return instance;
    }

    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "test";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "evan";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    return 
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