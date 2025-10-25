/*
** ETIB PROJECT, 2025
** evan
** File description:
** GraphicalContext
*/

#include "openxr/GraphicalContext.hpp"

evan::openxr::GraphicalContext::GraphicalContext(
    const GraphicalContextPropertiesXR &properties)
    : _XRinstance(properties._XRinstance), _XRsystemID(properties._XRsystemID) {
  createInstance();

  RenderingContextPropertiesXR renderingProperties{};
  renderingProperties._XRinstance = _XRinstance;
  renderingProperties._XRsystemID = _XRsystemID;
  renderingProperties._vulkanInstance = _instance;

  _renderingContext =
      std::make_shared<evan::openxr::RenderingContext>(renderingProperties);

  initializeSession();

  auto vulkanContext = _renderingContext->getVulkanContext();

  SwapchainContextCreationPropertiesXR swapchainProperties{};
  swapchainProperties._instance = _XRinstance;
  swapchainProperties._systemId = _XRsystemID;
  swapchainProperties._session = _XRsession;
  swapchainProperties._physicalDevice = vulkanContext->physicalDevice;
  swapchainProperties._device = vulkanContext->logicalDevice;
  swapchainProperties._msaaSamples = vulkanContext->msaaSamples;
  swapchainProperties._commandPool = vulkanContext->commandPool;
  swapchainProperties._graphicsQueue = vulkanContext->graphicsQueue;

  _swapchainContext =
      std::make_shared<evan::openxr::SwapchainContext>(swapchainProperties);
}

evan::openxr::GraphicalContext::~GraphicalContext() {}

void evan::openxr::GraphicalContext::initializeSession() {
  if (_XRsession != XR_NULL_HANDLE)
    return;

  std::shared_ptr<VulkanContext> vulkanContext =
      _renderingContext->getVulkanContext();
  if (vulkanContext == nullptr) {
    std::cerr << "Failed to get Vulkan context" << std::endl;
    return;
  }

  XrGraphicsBindingVulkan2KHR graphicsBinding{};

  graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR;
  graphicsBinding.next = nullptr;
  graphicsBinding.instance = _instance;
  graphicsBinding.device = vulkanContext->logicalDevice;
  graphicsBinding.queueFamilyIndex = vulkanContext->graphicsQueueFamilyIndex;
  graphicsBinding.queueIndex = 0;

  XrSessionCreateInfo sessionCreateInfo{};
  sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO;
  sessionCreateInfo.next = &graphicsBinding;
  sessionCreateInfo.systemId = _XRsystemID;

  if (xrCreateSession(_XRinstance, &sessionCreateInfo, &_XRsession) !=
      XR_SUCCESS) {
    std::cerr << "Failed to create OpenXR session" << std::endl;
    return;
  }
}

void evan::openxr::GraphicalContext::createVisualizedSpace() {
  XrReferenceSpaceCreateInfo spaceCreateInfo{};
  spaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
  spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
  spaceCreateInfo.poseInReferenceSpace = {{0, 0, 0, 1}, {0, 0, 0}};

  XrSpace space;
  if (xrCreateReferenceSpace(_XRsession, &spaceCreateInfo, &space) !=
      XR_SUCCESS) {
    std::cerr << "Failed to create reference space" << std::endl;
    return;
  }
  _XRvisualizedSpaces.push_back(space);
}

void evan::openxr::GraphicalContext::createInstance() {
  XrGraphicsRequirementsVulkan2KHR graphicsRequirements{};
  PFN_xrGetVulkanGraphicsRequirements2KHR xrGetVulkanGraphicsRequirements2KHR =
      nullptr;
  PFN_xrCreateVulkanInstanceKHR xrCreateVulkanInstanceKHR = nullptr;

  graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR;
  if (xrGetInstanceProcAddr(_XRinstance, "xrGetVulkanGraphicsRequirements2KHR",
                            reinterpret_cast<PFN_xrVoidFunction *>(
                                &xrGetVulkanGraphicsRequirements2KHR)) !=
      XR_SUCCESS) {
    std::cerr << "Failed to get xrGetVulkanGraphicsRequirements2KHR function"
              << std::endl;
    return;
  }
  if (xrGetVulkanGraphicsRequirements2KHR(
          _XRinstance, _XRsystemID, &graphicsRequirements) != XR_SUCCESS) {
    std::cerr << "Failed to get Vulkan graphics requirements" << std::endl;
    return;
  }
  if (xrGetInstanceProcAddr(_XRinstance, "xrCreateVulkanInstanceKHR",
                            reinterpret_cast<PFN_xrVoidFunction *>(
                                &xrCreateVulkanInstanceKHR)) != XR_SUCCESS) {
    std::cerr << "Failed to get xrCreateVulkanInstanceKHR function"
              << std::endl;
    return;
  }

  std::vector<const char *> layers{};
  std::vector<const char *> extensions{};

  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "test",
      .applicationVersion = 0u,
      .pEngineName = "evan",
      .engineVersion = 0u,
      .apiVersion = VK_API_VERSION_1_3,
  };

  VkInstanceCreateInfo vkCreateInfo{};
  vkCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  vkCreateInfo.pNext = nullptr;
  vkCreateInfo.flags = 0;
  vkCreateInfo.pApplicationInfo = &appInfo;
  vkCreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
  vkCreateInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();
  vkCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  vkCreateInfo.ppEnabledExtensionNames =
      extensions.empty() ? nullptr : extensions.data();

  XrVulkanInstanceCreateInfoKHR vkInstanceCreateInfo{};
  vkInstanceCreateInfo.type = XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR;
  vkInstanceCreateInfo.systemId = _XRsystemID;
  vkInstanceCreateInfo.createFlags = 0;
  vkInstanceCreateInfo.pfnGetInstanceProcAddr = &vkGetInstanceProcAddr;
  vkInstanceCreateInfo.vulkanCreateInfo = &vkCreateInfo;
  vkInstanceCreateInfo.vulkanAllocator = nullptr;

  VkResult result = VK_SUCCESS;
  if (xrCreateVulkanInstanceKHR(_XRinstance, &vkInstanceCreateInfo, &_instance,
                                &result) != XR_SUCCESS) {
    std::cerr << "Failed to create Vulkan instance" << std::endl;
    return;
  }
  if (result != VK_SUCCESS) {
    std::cerr << "Failed to create Vulkan instance: " << result << std::endl;
    return;
  }
}

std::vector<std::string>
evan::openxr::GraphicalContext::getInstanceExtensions() {
  return {XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME};
}
