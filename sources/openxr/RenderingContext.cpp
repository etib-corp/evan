/*
** ETIB PROJECT, 2025
** evan
** File description:
** RenderingContext
*/

#include "openxr/RenderingContext.hpp"

evan::openxr::RenderingContext::RenderingContext(
    const RenderingContextPropertiesXR &properties)
    : _XRinstance(properties._XRinstance), _XRsystemID(properties._XRsystemID),
      _vulkanInstance(properties._vulkanInstance) {
  pickPhysicalDevice(_vulkanInstance);
  createLogicalDevice();
  createCommandPool();
  _msaaSamples = getMaxUsableSampleCount();

  _vulkanContext = std::make_shared<VulkanContext>(
      _logicalDevice, _physicalDevice, _graphicsQueue, _commandPool,
      Utils::findQueueFamilies(_physicalDevice).graphicsFamily.value(),
      _msaaSamples); // ! builds only without this even though it's needed
}

evan::openxr::RenderingContext::~RenderingContext() {}

void evan::openxr::RenderingContext::pickPhysicalDevice(VkInstance instance) {
  if (_XRinstance == XR_NULL_HANDLE) {
    std::cerr << "OpenXR instance is not initialized" << std::endl;
    return;
  }

  if (_physicalDevice != VK_NULL_HANDLE) {
    return;
  }
  PFN_xrGetVulkanGraphicsDevice2KHR xrGetVulkanGraphicsDevice2KHR = nullptr;
  XrVulkanGraphicsDeviceGetInfoKHR graphicsDeviceGetInfo{};

  if (xrGetInstanceProcAddr(_XRinstance, "xrGetVulkanGraphicsDevice2KHR",
                            reinterpret_cast<PFN_xrVoidFunction *>(
                                &xrGetVulkanGraphicsDevice2KHR)) !=
      XR_SUCCESS) {
    std::cerr << "Failed to get xrGetVulkanGraphicsDevice2KHR function"
              << std::endl;
    return;
  }
  graphicsDeviceGetInfo.type = XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR;
  graphicsDeviceGetInfo.systemId = _XRsystemID;
  graphicsDeviceGetInfo.vulkanInstance = instance;

  if (xrGetVulkanGraphicsDevice2KHR(_XRinstance, &graphicsDeviceGetInfo,
                                    &_physicalDevice) != XR_SUCCESS) {
    std::cerr << "Failed to get Vulkan graphics device" << std::endl;
    return;
  }
}

void evan::openxr::RenderingContext::createLogicalDevice() {
  if (_physicalDevice == VK_NULL_HANDLE) {
    std::cerr << "Physical device is not initialized" << std::endl;
    return;
  }

  if (_logicalDevice != VK_NULL_HANDLE) {
    return;
  }

  PFN_xrCreateVulkanDeviceKHR xrCreateVulkanDeviceKHR = nullptr;

  if (xrGetInstanceProcAddr(_XRinstance, "xrCreateVulkanDeviceKHR",
                            reinterpret_cast<PFN_xrVoidFunction *>(
                                &xrCreateVulkanDeviceKHR)) != XR_SUCCESS) {
    std::cerr << "Failed to get xrCreateVulkanDeviceKHR function" << std::endl;
    return;
  }

  float queuePriority = 0.0f;
  VkDeviceQueueCreateInfo queueCreateInfo{};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueCount = 1;
  queueCreateInfo.pQueuePriorities = &queuePriority;

  Utils::QueueFamilyIndices queueFamilyIndices =
      Utils::findQueueFamilies(_physicalDevice);
  if (!queueFamilyIndices.isComplete()) {
    std::cerr << "Queue family indices are not complete" << std::endl;
    return;
  }
  queueCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  VkPhysicalDeviceFeatures features{};

  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
  deviceCreateInfo.pEnabledFeatures = &features;
  deviceCreateInfo.enabledExtensionCount = 0;
  deviceCreateInfo.ppEnabledExtensionNames = nullptr;
  deviceCreateInfo.enabledLayerCount = 0;
  deviceCreateInfo.ppEnabledLayerNames = nullptr;

  XrVulkanDeviceCreateInfoKHR vulkanDeviceCreateInfo{};
  vulkanDeviceCreateInfo.type = XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR;
  vulkanDeviceCreateInfo.next = &deviceCreateInfo;
  vulkanDeviceCreateInfo.systemId = _XRsystemID;
  vulkanDeviceCreateInfo.createFlags = 0;
  vulkanDeviceCreateInfo.pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
  vulkanDeviceCreateInfo.vulkanPhysicalDevice = _physicalDevice;
  vulkanDeviceCreateInfo.vulkanCreateInfo = &deviceCreateInfo;
  vulkanDeviceCreateInfo.vulkanAllocator = nullptr;

  VkResult result = VK_SUCCESS;
  if (xrCreateVulkanDeviceKHR(_XRinstance, &vulkanDeviceCreateInfo,
                              &_logicalDevice, &result) != XR_SUCCESS) {
    std::cerr << "Failed to create Vulkan device" << std::endl;
    return;
  }

  if (result != VK_SUCCESS) {
    std::cerr << "Failed to create Vulkan device: " << result << std::endl;
    return;
  }

  vkGetDeviceQueue(_logicalDevice, queueCreateInfo.queueFamilyIndex, 0,
                   &_graphicsQueue);
}
