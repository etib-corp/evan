/*
** ETIB PROJECT, 2025
** evan
** File description:
** Software
*/

#include "openxr/Software.hpp"

evan::openxr::Software::Software(
    const std::shared_ptr<PlatformData> &platformData) {
  _platform = std::make_shared<AndroidPlatform>(platformData);
  _graphicalContext = nullptr;

  createInstance();
  initializeSystem();
  GraphicalContextPropertiesXR properties;
  properties._XRinstance = _XRinstance;
  properties._XRsystemID = _XRsystemID;
  _graphicalContext =
      std::make_shared<evan::openxr::GraphicalContext>(properties);
}

evan::openxr::Software::~Software() {}

void evan::openxr::Software::createInstance() {
  if (_XRinstance != XR_NULL_HANDLE)
    return;
  std::vector<const char *> extensions{
      XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME};

  const std::vector<std::string> graphicsExtensions = {
      XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME};
  std::transform(graphicsExtensions.begin(), graphicsExtensions.end(),
                 std::back_inserter(extensions),
                 [](const std::string &ext) { return ext.c_str(); });

  XrInstanceCreateInfo createInfo{};
  createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
  createInfo.next = _platform->getInstanceCreateInfoAndroid();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.enabledExtensionNames = extensions.data();
  createInfo.enabledApiLayerCount = 0;
  createInfo.enabledApiLayerNames = nullptr;
  std::strcpy(createInfo.applicationInfo.applicationName, "evan");
  createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

  if (xrCreateInstance(&createInfo, &_XRinstance) != XR_SUCCESS) {
    std::cerr << "Failed to create OpenXR instance" << std::endl;
    return;
  }
}

void evan::openxr::Software::initializeSystem() {
  if (_XRsystemID != XR_NULL_SYSTEM_ID)
    return;

  XrSystemGetInfo systemInfo{};
  systemInfo.type = XR_TYPE_SYSTEM_GET_INFO;
  systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
  if (xrGetSystem(_XRinstance, &systemInfo, &_XRsystemID) != XR_SUCCESS) {
    std::cerr << "Failed to get OpenXR system ID" << std::endl;
    return;
  }
}
