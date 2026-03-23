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

////////////////////
// Public Methods //
////////////////////

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

void evan::XrPlatform::pollEvents(ADeviceBackend &deviceBackend)
{
    XrEventDataBuffer eventDataBuffer{XR_TYPE_EVENT_DATA_BUFFER};
    XrInstance instance = dynamic_cast<XrDeviceBackend&>(deviceBackend)._XrInstance;
    XrSession session = dynamic_cast<XrDeviceBackend&>(deviceBackend)._session;
    while (xrPollEvent(instance, &eventDataBuffer) == XR_SUCCESS) {
        switch (eventDataBuffer.type) {
            case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
                auto sessionStateChangedEvent = *reinterpret_cast<XrEventDataSessionStateChanged*>(&eventDataBuffer);
                processSessionStateChangedEvent(sessionStateChangedEvent, session);
                break;
            }
            case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
                _shouldClose = true;
                break;
            }
            default:
                break;
        }
        eventDataBuffer = {XR_TYPE_EVENT_DATA_BUFFER};
    }

}

const XrBaseInStructure* evan::XrPlatform::getInstanceCreateInfoAndroid() const
{
    #ifdef __ANDROID__
        return reinterpret_cast<const XrBaseInStructure*>(&_instanceCreateInfoAndroid);
    #else
        return nullptr;
    #endif
}

///////////////////////
// Protected Methods //
///////////////////////

void evan::XrPlatform::processSessionStateChangedEvent(const XrEventDataSessionStateChanged &eventData, XrSession session)
{
    switch (eventData.state) {
        case XR_SESSION_STATE_READY: {
            XrSessionBeginInfo sessionBeginInfo;
            sessionBeginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
            sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
            xrBeginSession(session, &sessionBeginInfo);
            _sessionRunning = true;
            break;
        }
        case XR_SESSION_STATE_STOPPING:
            xrEndSession(session);
            _sessionRunning = false;
            break;
        case XR_SESSION_STATE_EXITING:
            _shouldClose = true;
            break;
        case XR_SESSION_STATE_LOSS_PENDING:
            _shouldClose = true;
            break;
        default:
            break;
    }
}
