/*
** ETIB PROJECT, 2025
** evan
** File description:
** SwapchainContext
*/

#include "openxr/SwapchainContext.hpp"

evan::openxr::SwapchainContext::SwapchainContext(
    const SwapchainContextCreationPropertiesXR &properties)
    : _instance(properties._instance), _systemId(properties._systemId),
      _session(properties._session),
      _physicalDevice(properties._physicalDevice), _device(properties._device),
      _msaaSamples(properties._msaaSamples),
      _commandPool(properties._commandPool),
      _graphicsQueue(properties._graphicsQueue) {
  init();
}

evan::openxr::SwapchainContext::~SwapchainContext() {}

VkFormat evan::openxr::SwapchainContext::selectSwapchainFormat(
    const std::vector<int64_t> &swapchainFormats) {
  constexpr VkFormat kPreferredSwapchainFormats[] = {
      VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_UNORM,
      VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM};

  auto swapchainFormatIt =
      std::find_first_of(swapchainFormats.begin(), swapchainFormats.end(),
                         std::begin(kPreferredSwapchainFormats),
                         std::end(kPreferredSwapchainFormats));

  if (swapchainFormatIt == swapchainFormats.end()) {
    std::cerr << "SwapchainContext: No preferred swapchain format found, using "
                 "first available format"
              << std::endl;
    return static_cast<VkFormat>(swapchainFormats[0]);
  }
  return static_cast<VkFormat>(*swapchainFormatIt);
}

void evan::openxr::SwapchainContext::init() {
  if (_instance == XR_NULL_HANDLE || _session == XR_NULL_HANDLE) {
    std::cerr << "SwapchainContext: Invalid instance or session" << std::endl;
    return;
  }

  uint32_t swapchainFormatCount = 0;
  xrEnumerateSwapchainFormats(_session, 0, &swapchainFormatCount, nullptr);
  if (swapchainFormatCount == 0) {
    std::cerr << "SwapchainContext: No swapchain formats available"
              << std::endl;
    return;
  }
  std::vector<int64_t> swapchainFormats(swapchainFormatCount);
  xrEnumerateSwapchainFormats(_session, swapchainFormatCount,
                              &swapchainFormatCount, swapchainFormats.data());

  uint32_t viewCount = 0;
  xrEnumerateViewConfigurationViews(_instance, _systemId,
                                    XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
                                    0, &viewCount, nullptr);
  if (viewCount == 0) {
    std::cerr << "SwapchainContext: No view configurations available"
              << std::endl;
    return;
  }
  _viewsConfigurations.resize(viewCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW, nullptr, 0, 0, 0, 0, 0, 0});
  xrEnumerateViewConfigurationViews(
      _instance, _systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
      viewCount, &viewCount, _viewsConfigurations.data());
  _views.resize(viewCount, {XR_TYPE_VIEW, nullptr, {}, {}});

  _swapchainColorFormat = selectSwapchainFormat(swapchainFormats);

  createRenderPass();
  createGraphicsPipeline();

  for (const auto &viewConfig : _viewsConfigurations) {
    XrSwapchainCreateInfo swapchainCreateInfo = {};
    swapchainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
    swapchainCreateInfo.arraySize = 1;
    swapchainCreateInfo.format = _swapchainColorFormat;
    swapchainCreateInfo.width = viewConfig.recommendedImageRectWidth;
    swapchainCreateInfo.height = viewConfig.recommendedImageRectHeight;
    swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT |
                                     XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
    ASwapchain<XrSwapchain> swapchain;
    swapchain._height = viewConfig.recommendedImageRectHeight;
    swapchain._width = viewConfig.recommendedImageRectWidth;

    XrResult result =
        xrCreateSwapchain(_session, &swapchainCreateInfo, &swapchain.swapchain);
    if (result != XR_SUCCESS) {
      std::cerr << "SwapchainContext: Failed to create swapchain, error: "
                << result << std::endl;
      return;
    }
    _swapchain.push_back(swapchain);
    std::shared_ptr<evan::openxr::SwapChainImage> swapchainImage =
        createSwapchainImage(swapchainCreateInfo, swapchain);

    _swapchainImages[swapchain.swapchain] = swapchainImage;

    XrSwapchainImageBaseHeader *image = swapchainImage->getFirstImagePointer();

    xrEnumerateSwapchainImages(swapchain.swapchain,
                               swapchainImage->_swapchainImages.size(), nullptr,
                               image);
  }
}

std::shared_ptr<evan::openxr::SwapChainImage>
evan::openxr::SwapchainContext::createSwapchainImage(
    XrSwapchainCreateInfo &swapchainCreateInfo,
    ASwapchain<XrSwapchain> &swapchain) {
  uint32_t imageCount = 0;
  xrEnumerateSwapchainImages(swapchain.swapchain, 0, &imageCount, nullptr);

  SwapchainImageCreationPropertiesXR properties;
  properties._capacity = imageCount;
  properties._device = _device;
  properties._physicalDevice = _physicalDevice;
  properties._swapchainCreateInfo = swapchainCreateInfo;
  properties._commandPool = _commandPool;
  properties._graphicsQueue = _graphicsQueue;

  std::shared_ptr<evan::openxr::SwapChainImage> swapchainImage =
      std::make_shared<SwapChainImage>();
  swapchainImage->init(properties);

  return swapchainImage;
}

void evan::openxr::SwapChainImage::init(
    const SwapchainImageCreationPropertiesXR &properties) {
  _device = properties._device;
  _physicalDevice = properties._physicalDevice;

  _commandPool = properties._commandPool;
  _graphicsQueue = properties._graphicsQueue;

  _swapchainImageFormat =
      static_cast<VkFormat>(properties._swapchainCreateInfo.format);
  _swapchainExtent = {properties._swapchainCreateInfo.width,
                      properties._swapchainCreateInfo.height};

  _swapchainImages.resize(properties._capacity);
  _swapchainImageViews.resize(properties._capacity);
  _swapchainFrameBuffers.resize(properties._capacity);

  _viewport = {.x = 0,
               .y = static_cast<float>(_swapchainExtent.height),
               .width = static_cast<float>(_swapchainExtent.width),
               .height = -static_cast<float>(_swapchainExtent.height),
               .minDepth = 0.0f,
               .maxDepth = 1.0f};
  _scissor.extent = {.width = _swapchainExtent.width,
                     .height = _swapchainExtent.height};

  for (auto &image : _swapchainImages) {
    image.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN2_KHR;
  }
}

void evan::openxr::SwapChainImage::createColorResources() {
  Utils::CreateImageProperties properties = {
      ._logicalDevice = _device,
      ._physicalDevice = _physicalDevice,
      ._width = _swapchainExtent.width,
      ._height = _swapchainExtent.height,
      ._mipLevels = 1,
      ._numSamples = VK_SAMPLE_COUNT_1_BIT,
      ._format = _swapchainImageFormat,
      ._tiling = VK_IMAGE_TILING_OPTIMAL,
      ._usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      ._properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      ._image = _colorImage,
      ._imageMemory = _depthImageMemory,
  };
  Utils::createImage(properties);

  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = _colorImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = _swapchainImageFormat;
  viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

  if (vkCreateImageView(_device, &viewInfo, nullptr, &_colorImageView) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create image view");
  }

  Utils::TransitionImageLayoutProperties transitionProperties = {
      ._logicalDevice = _device,
      ._commandPool = _commandPool,
      ._graphicsQueue = _graphicsQueue,
      ._image = _colorImage,
      ._format = _swapchainImageFormat,
      ._oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      ._newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      ._mipLevels = 1};
  Utils::transitionImageLayout(transitionProperties);
}

void evan::openxr::SwapChainImage::createDepthResources() {
  VkFormat depthFormat = Utils::findSupportedDepthFormat(_physicalDevice);
  Utils::CreateImageProperties properties = {
      ._logicalDevice = _device,
      ._physicalDevice = _physicalDevice,
      ._width = _swapchainExtent.width,
      ._height = _swapchainExtent.height,
      ._mipLevels = 1,
      ._numSamples = VK_SAMPLE_COUNT_1_BIT,
      ._format = depthFormat,
      ._tiling = VK_IMAGE_TILING_OPTIMAL,
      ._usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      ._properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      ._image = _depthImage,
      ._imageMemory = _depthImageMemory};
  Utils::createImage(properties);

  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = _depthImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = depthFormat;
  viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  if (vkCreateImageView(_device, &viewInfo, nullptr, &_depthImageView) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create image view");
  }

  Utils::TransitionImageLayoutProperties transitionProperties = {
      ._logicalDevice = _device,
      ._commandPool = _commandPool,
      ._graphicsQueue = _graphicsQueue,
      ._image = _depthImage,
      ._format = depthFormat,
      ._oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      ._newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      ._mipLevels = 1};
  Utils::transitionImageLayout(transitionProperties);
}

void evan::openxr::SwapChainImage::createFrameBuffers() {
  for (size_t i = 0; i < _swapchainImageViews.size(); i++) {
    VkImageView attachments[] = {_colorImageView, _depthImageView,
                                 _swapchainImageViews[i]};

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = nullptr; // Set your render pass here
    framebufferInfo.attachmentCount = 2;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = _swapchainExtent.width;
    framebufferInfo.height = _swapchainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr,
                            &_swapchainFrameBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffer");
    }
  }

  Utils::TransitionImageLayoutProperties transitionProperties = {
      ._logicalDevice = _device,
      ._commandPool = _commandPool,
      ._graphicsQueue = _graphicsQueue,
      ._image = _colorImage,
      ._format = _swapchainImageFormat,
      ._oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      ._newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      ._mipLevels = 1};
  Utils::transitionImageLayout(transitionProperties);
}

XrSwapchainImageBaseHeader *
evan::openxr::SwapChainImage::getFirstImagePointer() {
  if (_swapchainImages.empty()) {
    return nullptr;
  }
  return reinterpret_cast<XrSwapchainImageBaseHeader *>(&_swapchainImages[0]);
}

void evan::openxr::SwapchainContext::createRenderPass() {
  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = evan::Utils::findDepthFormat(_physicalDevice);
  depthAttachment.samples = _msaaSamples;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depthAttachment.finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = _swapchainColorFormat;
  colorAttachment.samples = _msaaSamples;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription colorAttachmentResolve{};
  colorAttachmentResolve.format = _swapchainColorFormat;
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
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
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
  if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create render pass");
  }
}

void evan::openxr::SwapchainContext::createGraphicsPipeline() {
  auto vertShaderCode = Utils::readFile("shaders/vert.spv");
  auto fragShaderCode = Utils::readFile("shaders/frag.spv");

  VkShaderModule vertShaderModule =
      Utils::createShaderModule(_device, vertShaderCode);
  VkShaderModule fragShaderModule =
      Utils::createShaderModule(_device, fragShaderCode);

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
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexBindingDescriptions = nullptr;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = nullptr;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = nullptr; // Viewports will be set dynamically
  viewportState.scissorCount = 1;
  viewportState.pScissors = nullptr; // Scissors will be set dynamically

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.rasterizationSamples = _msaaSamples;

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
  pipelineLayoutInfo.setLayoutCount = 0; // No descriptor set layout for now
  pipelineLayoutInfo.pushConstantRangeCount = 0; // No push constants for now
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;

  if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr,
                             &_pipelineLayout) != VK_SUCCESS) {
    std::cerr << "Failed to create pipeline layout" << std::endl;
    return;
  }

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

  if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                nullptr, &_graphicsPipeline) != VK_SUCCESS) {
    std::cerr << "Failed to create graphics pipeline" << std::endl;
    return;
  }
}
