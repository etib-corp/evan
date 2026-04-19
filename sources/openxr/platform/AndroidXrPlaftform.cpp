/*
** ETIB PROJECT, 2026
** evan
** File description:
** AndroidXrPlaftform
*/

#include "openxr/platform/AndroidXrPlatform.hpp"
#include "openxr/XrDeviceBackend.hpp"

evan::AndroidXrPlatform::AndroidXrPlatform(
    const AndroidPlatformData &platformData)
{
	_platformData = platformData;
#ifdef __ANDROID__
	PFN_xrInitializeLoaderKHR initializeLoader = nullptr;

	if (xrGetInstanceProcAddr(
			XR_NULL_HANDLE, "xrInitializeLoaderKHR",
			reinterpret_cast<PFN_xrVoidFunction *>(&initializeLoader))
		== XR_SUCCESS) {
		XrLoaderInitInfoAndroidKHR loaderInitInfoAndroid = {};
		loaderInitInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
		loaderInitInfoAndroid.next = nullptr;
		loaderInitInfoAndroid.applicationVM		 = platformData.applicationVM;
		loaderInitInfoAndroid.applicationContext = platformData.applicationActivity;
		initializeLoader(
			reinterpret_cast<const XrLoaderInitInfoBaseHeaderKHR *>(
				&loaderInitInfoAndroid));
	}
	_instanceCreateInfoAndroid = { XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR };
	_instanceCreateInfoAndroid.applicationVM	   = platformData.applicationVM;
	_instanceCreateInfoAndroid.applicationActivity = platformData.applicationActivity;
#endif
}

////////////////////
// Public Methods //
////////////////////

std::vector<std::string> evan::AndroidXrPlatform::getRequiredInstanceExtensions() const
{
	std::vector<std::string> extensions = {
		XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME,
        #ifdef XR_USE_PLATFORM_ANDROID
            XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME
        #endif
	};
    return extensions;
}

const XrBaseInStructure *evan::AndroidXrPlatform::getInstanceCreateInfo() const
{
    #ifdef __ANDROID__
    return reinterpret_cast<const XrBaseInStructure *>(&_instanceCreateInfoAndroid);
    #else
    return nullptr;
    #endif
}
