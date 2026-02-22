/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrPlatform
*/

#include "openxr/XrPlatform.hpp"

evan::XrPlatform::XrPlatform(const AndroidPlatformData &data)
{
    PFN_xrInitializeLoaderKHR initializeLoader = nullptr;

    XrLoaderInitInfoAndroidKHR loaderInitInfo{ XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR };
    loaderInitInfo.applicationVM = data.applicationVM;
    loaderInitInfo.applicationActivity = data.applicationActivity;
    if (xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", reinterpret_cast<PFN_xrVoidFunction*>(&initializeLoader)) != XR_SUCCESS) {
        std::cerr << "Failed to get xrInitializeLoaderKHR function pointer." << std::endl;
        return;
    }
   
    initializeLoader(reinterpret_cast<const XrLoaderInitInfoBaseHeaderKHR*>(&loaderInitInfo));
}

evan::XrPlatform::XrPlatform()
{
}

evan::XrPlatform::~XrPlatform()
{
}

std::vector<std::string> evan::XrPlatform::getRequiredInstanceExtensions() const
{
    std::vector<std::string> extensions = {
        XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME, 
    };

    #ifdef __ANDROID__
        extensions.push_back(XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME);
    #endif

    return extensions;
}

bool evan::XrPlatform::shouldClose() const
{
    return _shouldClose;
}

void evan::XrPlatform::pollEvents()
{
    // TODO: Implement event polling for OpenXR, such as handling session state changes, input events, etc.
}

const XrBaseInStructure* evan::XrPlatform::getInstanceCreateInfoAndroid() const
{
    #ifdef __ANDROID__
        static XrInstanceCreateInfoAndroidKHR androidCreateInfo{ XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR };
        // Populate androidCreateInfo with necessary Android-specific instance creation information
        return reinterpret_cast<const XrBaseInStructure*>(&androidCreateInfo);
    #else
        return nullptr;
    #endif
}