/*
** ETIB PROJECT, 2025
** evan
** File description:
** SwapchainContext
*/

#include "glfw/SwapchainContext.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

////////////////////
// Static methods //
////////////////////

/**
 * @brief Default debug callback function for Vulkan validation layers.
 *
 * This function is called whenever a validation layer generates a debug
 * message. It outputs the message to the standard error stream.
 *
 * @param messageSeverity Specifies the severity of the message (e.g., verbose,
 * info, warning, or error).
 * @param messageType Specifies the type of the message (e.g., general,
 * validation, or performance).
 * @param pCallbackData Pointer to a structure containing details about the
 * debug message.
 * @param pUserData Pointer to user-defined data passed during the creation of
 * the debug messenger.
 *
 * @return Always returns VK_FALSE, indicating that the Vulkan call that
 * triggered the callback should not be aborted.
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL
defaultDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                     void *pUserData) {
  std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

////////////////////
// Public methods //
////////////////////

evan::glfw::SwapchainContext::SwapchainContext(
    const SwapchainContextCreationProperties &properties) {
  struct TextureImageCreationProperties textureImageProperties = {
      properties._physicalDevice, properties._logicalDevice,
      properties._commandPool, properties._msaaSamples,
      properties._graphicsQueue};

  this->_creationProperties = properties;

  this->setupDebugMessenger(properties._instance);

  this->init(properties._surface, properties._physicalDevice,
             properties._logicalDevice, properties._window);

  this->createDescriptorSetLayout(properties._logicalDevice);

  this->createRenderPass();

  this->createColorResources(properties._logicalDevice,
                             properties._physicalDevice,
                             properties._msaaSamples);
  this->createDepthResources(textureImageProperties);
  this->createFramebuffers(properties._logicalDevice, _renderPass);

  this->createUniformBuffers(properties._logicalDevice,
                             properties._physicalDevice);
  this->createDescriptorPool(properties._logicalDevice);
  this->createDescriptorSets(properties._logicalDevice,
                             {{this->_textureImageView.begin()->second,
                               this->_textureSampler.begin()->second}});

  this->createGraphicsPipeline();
}

evan::glfw::SwapchainContext::~SwapchainContext() {}

void evan::glfw::SwapchainContext::recreate(
    const SwapchainContextCreationProperties &properties) {
  int width = 0;
  int height = 0;
  struct TextureImageCreationProperties textureImageProperties = {
      properties._physicalDevice, properties._logicalDevice,
      properties._commandPool, properties._msaaSamples,
      properties._graphicsQueue};

  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(properties._window, &width, &height);
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(properties._logicalDevice);

  this->cleanup(properties._logicalDevice);

  this->init(properties._surface, properties._physicalDevice,
             properties._logicalDevice, properties._window);

  this->createColorResources(properties._logicalDevice,
                             properties._physicalDevice,
                             properties._msaaSamples);
  this->createDepthResources(textureImageProperties);
  this->createFramebuffers(properties._logicalDevice, _renderPass);
}

///////////////////////
// Protected methods //
///////////////////////

void evan::glfw::SwapchainContext::setupDebugMessenger(VkInstance instance) {
  if (!enableValidationLayers)
    return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo;

  Utils::populateDebugMessengerCreateInfo(createInfo, defaultDebugCallback);
  if (Utils::createDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
                                          &_debugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("Failed to set up debug messenger !");
  }
}

void evan::glfw::SwapchainContext::createDescriptorPool(
    VkDevice logicalDevice) {
  std::array<VkDescriptorPoolSize, 2> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr,
                             &_descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor pool!");
  }
}

void evan::glfw::SwapchainContext::createDescriptorSetLayout(
    VkDevice logicalDevice) {
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding,
                                                          samplerLayoutBinding};
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr,
                                  &_descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

void evan::glfw::SwapchainContext::createDescriptorSets(
    VkDevice logicalDevice,
    std::map<VkImageView, VkSampler> imagesViewsAndSamplers) {
  for (const auto [imageView, sampler] : imagesViewsAndSamplers) {
    this->createSingleDescriptorSets(logicalDevice, imageView, sampler);
  }
}

void evan::glfw::SwapchainContext::init(VkSurfaceKHR surface,
                                         VkPhysicalDevice physicalDevice,
                                         VkDevice logicalDevice,
                                         GLFWwindow *window) {
  Utils::SwapChainSupportDetails swapChainSupport =
      Utils::querySwapChainSupport(physicalDevice, surface);

  VkSurfaceFormatKHR surfaceFormat =
      this->chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode =
      this->chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent =
      this->chooseSwapExtent(swapChainSupport.capabilities, window);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  Utils::QueueFamilyIndices indices =
      Utils::findQueueFamilies(physicalDevice, surface);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                   indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr,
                           &_swapchain.swapchain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  vkGetSwapchainImagesKHR(logicalDevice, _swapchain.swapchain, &imageCount,
                          nullptr);
  _swapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(logicalDevice, _swapchain.swapchain, &imageCount,
                          _swapchainImages.data());

  _swapchainColorFormat = surfaceFormat.format;
  _swapchainExtent = extent;

  this->createImageViews(logicalDevice);
}

void evan::glfw::SwapchainContext::createImageViews(VkDevice logicalDevice) {
  _imageViews.resize(_swapchainImages.size());

  for (uint32_t i = 0; i < _swapchainImages.size(); i++) {
    _imageViews[i] = Utils::createImageView(
        _swapchainImages[i], _swapchainColorFormat, VK_IMAGE_ASPECT_COLOR_BIT,
        logicalDevice, _mipLevels);
  }
}

void evan::glfw::SwapchainContext::createFramebuffers(
    VkDevice logicalDevice, VkRenderPass renderPass) {
  _swapchainFramebuffers.resize(_imageViews.size());

  for (size_t i = 0; i < _imageViews.size(); i++) {
    std::array<VkImageView, 3> attachments = {_colorImageView, _depthImageView,
                                              _imageViews[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = _swapchainExtent.width;
    framebufferInfo.height = _swapchainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr,
                            &_swapchainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffer!");
    }
  }
}

void evan::glfw::SwapchainContext::cleanup(VkDevice logicalDevice) {
  for (auto framebuffer : _swapchainFramebuffers) {
    vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
  }

  for (auto imageView : _imageViews) {
    vkDestroyImageView(logicalDevice, imageView, nullptr);
  }

  vkDestroySwapchainKHR(logicalDevice, _swapchain.swapchain, nullptr);
}

void evan::glfw::SwapchainContext::createTextureImage(
    const std::string &texturePath,
    const TextureImageCreationProperties &properties) {
  int texWidth = 0;
  int texHeight = 0;
  int texChannels = 0;
  stbi_uc *pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight,
                              &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  if (!pixels) {
    throw std::runtime_error("Failed to load texture image !");
  }

  _mipLevels = static_cast<uint32_t>(
                   std::floor(std::log2(std::max(texWidth, texHeight)))) +
               1;

  Utils::CreateBufferProperties stagingBufferProperties = {
      ._logicalDevice = properties._logicalDevice,
      ._physicalDevice = properties._physicalDevice,
      ._size = imageSize,
      ._usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      ._buffer = stagingBuffer,
      ._bufferMemory = stagingBufferMemory};

  Utils::createBuffer(stagingBufferProperties);

  void *data;
  vkMapMemory(properties._logicalDevice, stagingBufferMemory, 0, imageSize, 0,
              &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(properties._logicalDevice, stagingBufferMemory);

  stbi_image_free(pixels);

  Utils::CreateImageProperties imageProperties = {
      ._logicalDevice = properties._logicalDevice,
      ._physicalDevice = properties._physicalDevice,
      ._width = (uint32_t)texWidth,
      ._height = (uint32_t)texHeight,
      ._mipLevels = _mipLevels,
      ._format = VK_FORMAT_R8G8B8A8_SRGB,
      ._tiling = VK_IMAGE_TILING_OPTIMAL,
      ._usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      ._image = _textureImage[texturePath],
      ._imageMemory = _textureImageMemory};
  Utils::createImage(imageProperties);

  Utils::TransitionImageLayoutProperties transitionProperties = {
      ._logicalDevice = properties._logicalDevice,
      ._commandPool = properties._commandPool,
      ._graphicsQueue = properties._graphicsQueue,
      ._image = _textureImage[texturePath],
      ._format = VK_FORMAT_R8G8B8A8_SRGB,
      ._oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      ._newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      ._mipLevels = _mipLevels};
  Utils::transitionImageLayout(transitionProperties);

  Utils::CopyBufferToImageProperties copyProperties = {
      ._logicalDevice = properties._logicalDevice,
      ._commandPool = properties._commandPool,
      ._graphicsQueue = properties._graphicsQueue,
      ._buffer = stagingBuffer,
      ._image = _textureImage[texturePath],
      ._width = (uint32_t)texWidth,
      ._height = (uint32_t)texHeight};
  Utils::copyBufferToImage(copyProperties);

  Utils::GenerateMipmapsProperties propertiesMipmap = {
      ._physicalDevice = properties._physicalDevice,
      ._logicalDevice = properties._logicalDevice,
      ._commandPool = properties._commandPool,
      ._graphicsQueue = properties._graphicsQueue,
      ._image = _textureImage[texturePath],
      ._mipLevels = _mipLevels,
      ._texWidth = (uint32_t)texWidth,
      ._texHeight = (uint32_t)texHeight,
      ._imageFormat = VK_FORMAT_R8G8B8A8_SRGB};
  Utils::generateMipmaps(propertiesMipmap);

  vkDestroyBuffer(properties._logicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(properties._logicalDevice, stagingBufferMemory, nullptr);
}

void evan::glfw::SwapchainContext::createTextureImageView(
    VkDevice logicalDevice) {
  for (const auto &[textureName, textureImage] : _textureImage) {
    _textureImageView[textureName] = Utils::createImageView(
        textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT,
        logicalDevice, _mipLevels);
  }
}

void evan::glfw::SwapchainContext::createTextureSampler(
    VkDevice logicalDevice, VkPhysicalDevice physicalDevice,
    const std::string &textureName, VkSamplerCreateInfo samplerInfo) {
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);

  // Check if samplerInfo is "empty" by testing its sType field.
  // If sType is not set, it's likely uninitialized.
  if (samplerInfo.sType == 0) {
    samplerInfo = this->getDefaultSamplerInfo(properties);
  }

  if (vkCreateSampler(logicalDevice, &samplerInfo, nullptr,
                      &_textureSampler[textureName]) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create texture sampler for " +
                             textureName + " !");
  }
}

void evan::glfw::SwapchainContext::createRenderPass() {
  auto swapchainSupport = evan::Utils::querySwapChainSupport(
      _creationProperties._physicalDevice, _creationProperties._surface);
  auto swapchainFormat =
      this->chooseSwapSurfaceFormat(swapchainSupport.formats);

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swapchainFormat.format;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.samples = _creationProperties._msaaSamples;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format =
      evan::Utils::findDepthFormat(_creationProperties._physicalDevice);
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depthAttachment.samples = _creationProperties._msaaSamples;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription colorAttachmentResolve{};
  colorAttachmentResolve.format = swapchainFormat.format;
  colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentResolveRef{};
  colorAttachmentResolveRef.attachment = 2;
  colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;
  subpass.pResolveAttachments = &colorAttachmentResolveRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 3> attachments = {
      colorAttachment, depthAttachment, colorAttachmentResolve};
  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(_creationProperties._logicalDevice, &renderPassInfo,
                         nullptr, &_renderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void evan::glfw::SwapchainContext::createGraphicsPipeline() {
  auto vertShaderCode = Utils::readFile("shaders/vert.spv");
  auto fragShaderCode = Utils::readFile("shaders/frag.spv");

  VkShaderModule vertShaderModule = Utils::createShaderModule(
      _creationProperties._logicalDevice, vertShaderCode);
  VkShaderModule fragShaderModule = Utils::createShaderModule(
      _creationProperties._logicalDevice, fragShaderCode);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescriptions = Vertex::getAttributeDescriptions();

  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = _creationProperties._msaaSamples;
  multisampling.sampleShadingEnable = VK_TRUE;
  multisampling.minSampleShading = .2f;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                               VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

  if (vkCreatePipelineLayout(_creationProperties._logicalDevice,
                             &pipelineLayoutInfo, nullptr,
                             &_pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout !");
  }

  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds = 0.0f;
  depthStencil.maxDepthBounds = 1.0f;
  depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.front = {};
  depthStencil.back = {};

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = _pipelineLayout;
  pipelineInfo.renderPass = _renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.pDepthStencilState = &depthStencil;

  if (vkCreateGraphicsPipelines(_creationProperties._logicalDevice,
                                VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                &_graphicsPipeline) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create graphics pipeline !");
  }

  vkDestroyShaderModule(_creationProperties._logicalDevice, fragShaderModule,
                        nullptr);
  vkDestroyShaderModule(_creationProperties._logicalDevice, vertShaderModule,
                        nullptr);
}

/////////////////////
// Private methods //
/////////////////////

VkSurfaceFormatKHR evan::glfw::SwapchainContext::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }
  return availableFormats[0];
}

VkPresentModeKHR evan::glfw::SwapchainContext::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D evan::glfw::SwapchainContext::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window) {
  int width = 0;
  int height = 0;

  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  glfwGetFramebufferSize(window, &width, &height);

  VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                             static_cast<uint32_t>(height)};

  actualExtent.width =
      std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                 capabilities.maxImageExtent.width);
  actualExtent.height =
      std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                 capabilities.maxImageExtent.height);

  return actualExtent;
}

void evan::glfw::SwapchainContext::createColorResources(
    VkDevice logicalDevice, VkPhysicalDevice physicalDevice,
    VkSampleCountFlagBits msaaSamples) {
  VkFormat colorFormat = _swapchainColorFormat;
  Utils::CreateImageProperties imageProperties = {
      ._logicalDevice = logicalDevice,
      ._physicalDevice = physicalDevice,
      ._width = _swapchainExtent.width,
      ._height = _swapchainExtent.height,
      ._mipLevels = 1, // No mipmaps for color attachment
      ._format = colorFormat,
      ._tiling = VK_IMAGE_TILING_OPTIMAL,
      ._usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      ._image = _colorImage,
      ._imageMemory = _colorImageMemory};

  Utils::createImage(imageProperties);
  _colorImageView = Utils::createImageView(_colorImage, colorFormat,
                                           VK_IMAGE_ASPECT_COLOR_BIT,
                                           logicalDevice, _mipLevels);
}

void evan::glfw::SwapchainContext::createDepthResources(
    const TextureImageCreationProperties &properties) {
  VkFormat depthFormat = Utils::findDepthFormat(properties._physicalDevice);
  Utils::CreateImageProperties depthImageProperties = {
      ._logicalDevice = properties._logicalDevice,
      ._physicalDevice = properties._physicalDevice,
      ._width = _swapchainExtent.width,
      ._height = _swapchainExtent.height,
      ._mipLevels = 1, // No mipmaps for depth attachment
      ._format = depthFormat,
      ._tiling = VK_IMAGE_TILING_OPTIMAL,
      ._usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      ._image = _depthImage,
      ._imageMemory = _depthImageMemory};

  Utils::createImage(depthImageProperties);
  _depthImageView = Utils::createImageView(
      _depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT,
      properties._logicalDevice, _mipLevels);

  Utils::TransitionImageLayoutProperties transitionProperties = {
      ._logicalDevice = properties._logicalDevice,
      ._commandPool = properties._commandPool,
      ._graphicsQueue = properties._graphicsQueue,
      ._image = _depthImage,
      ._format = depthFormat,
      ._oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      ._newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      ._mipLevels = 1 // No mipmaps for depth attachment
  };
  Utils::transitionImageLayout(transitionProperties);
}

VkSamplerCreateInfo evan::glfw::SwapchainContext::getDefaultSamplerInfo(
    const VkPhysicalDeviceProperties &properties) {
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = static_cast<float>(_mipLevels);
  return samplerInfo;
}

void evan::glfw::SwapchainContext::createSingleDescriptorSets(
    VkDevice logicalDevice, VkImageView textureImageView,
    VkSampler textureSampler) {
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                             _descriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  allocInfo.pSetLayouts = layouts.data();

  _descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(logicalDevice, &allocInfo,
                               _descriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor sets !");
  }
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = _uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureImageView;
    imageInfo.sampler = textureSampler;

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = _descriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = _descriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(logicalDevice,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}

void evan::glfw::SwapchainContext::createUniformBuffers(
    VkDevice logicalDevice, VkPhysicalDevice physicalDevice) {
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  _uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  _uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
  _uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    Utils::CreateBufferProperties bufferProperties = {
        ._logicalDevice = logicalDevice,
        ._physicalDevice = physicalDevice,
        ._size = bufferSize,
        ._usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        ._buffer = _uniformBuffers[i],
        ._bufferMemory = _uniformBuffersMemory[i]};

    Utils::createBuffer(bufferProperties);
    vkMapMemory(logicalDevice, _uniformBuffersMemory[i], 0, bufferSize, 0,
                &_uniformBuffersMapped[i]);
  }
}
