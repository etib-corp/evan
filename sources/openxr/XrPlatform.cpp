/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrPlatform
*/

#include "openxr/XrPlatform.hpp"

evan::XrPlatform::XrPlatform(const AndroidPlatformData &data)
{
    #ifdef __ANDROID__
    PFN_xrInitializeLoaderKHR initializeLoader = nullptr;

    if (xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", reinterpret_cast<PFN_xrVoidFunction*>(&initializeLoader)) == XR_SUCCESS) {
        XrLoaderInitInfoAndroidKHR loaderInitInfoAndroid = {};
        loaderInitInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
        loaderInitInfoAndroid.next = nullptr;
        loaderInitInfoAndroid.applicationVM = data.applicationVM;
        loaderInitInfoAndroid.applicationContext = data.applicationActivity;
        initializeLoader(reinterpret_cast<const XrLoaderInitInfoBaseHeaderKHR *>(&loaderInitInfoAndroid));
    }
    _instanceCreateInfoAndroid = {XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR};
    _instanceCreateInfoAndroid.applicationVM = data.applicationVM;
    _instanceCreateInfoAndroid.applicationActivity = data.applicationActivity;
    #endif
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
        return reinterpret_cast<const XrBaseInStructure*>(&_instanceCreateInfoAndroid);
    #else
        return nullptr;
    #endif
}