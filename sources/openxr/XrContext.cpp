/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrContext
*/

#include "openxr/XrContext.hpp"

evan::XrContext::XrContext(const XrPlatform &platform)
{
    createInstance(platform);
    initializeSystem();
}

evan::XrContext::~XrContext()
{
    if (_instance != XR_NULL_HANDLE) {
        xrDestroyInstance(_instance);
    }
}

void evan::XrContext::createInstance(const XrPlatform &platform)
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

    #ifdef __ANDROID__
	createInfo.next = platform.getInstanceCreateInfoAndroid();
    #else
        createInfo.next = nullptr;
    #endif

    XrResult result = xrCreateInstance(&createInfo, &_instance);
    if (result != XR_SUCCESS) {
        // TODO: Throw an exception or handle the error appropriately
        std::cerr << "Failed to create OpenXR instance: " << result << std::endl;
        return;
    }
}

void evan::XrContext::initializeSystem()
{
    XrSystemGetInfo systemInfo{ XR_TYPE_SYSTEM_GET_INFO };
    systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    XrResult result = xrGetSystem(_instance, &systemInfo, &_systemId);
    if (result != XR_SUCCESS) {
        // TODO: Throw an exception or handle the error appropriately
        std::cerr << "Failed to get OpenXR system: " << result << std::endl;
        return;
    }
}