/*
** ETIB PROJECT, 2026
** evan
** File description:
** Renderer
*/

#include "Renderer.hpp"

evan::Renderer::Renderer(VkDevice device, VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples)
{
	this->_currentFrameIndex = 0;

	this->createDescriptorSetLayout(device);
	this->createGraphicsPipeline(device, renderPass, msaaSamples);
	this->createDescriptorPool(device, 1000); // TODO: Change this with the AssetManager when it will be implemented
}

evan::Renderer::~Renderer()
{
}

void evan::Renderer::createDescriptorSetLayout(VkDevice device)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void evan::Renderer::createGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples)
{
	auto vertShaderFile = g_assetManager->get("shaders/vert.spv");
	auto fragShaderFile = g_assetManager->get("shaders/frag.spv");

	std::string vertShaderString = vertShaderFile->content();
	std::string fragShaderString = fragShaderFile->content();

	std::vector<uint32_t> vertShaderCode(vertShaderString.size() / sizeof(uint32_t));
	std::vector<uint32_t> fragShaderCode(fragShaderString.size() / sizeof(uint32_t));

    Shader shader(vertShaderCode, fragShaderCode, device);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
	vertShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = shader.getVertexShaderModule();
	vertShaderStageInfo.pName  = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
	fragShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = shader.getFragmentShaderModule();
	fragShaderStageInfo.pName  = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo,
													   fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
	vertexInputInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription	   = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount =
		static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions	 = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
	inputAssembly.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology				 = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount	= 1;

	VkPipelineRasterizationStateCreateInfo rasterizer {};
	rasterizer.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable		   = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode			   = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth			   = 1.0f;
	rasterizer.cullMode				   = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace			   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable		   = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling {};
	multisampling.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable  = VK_FALSE;
	multisampling.rasterizationSamples = msaaSamples;
	multisampling.sampleShadingEnable  = VK_TRUE;
	multisampling.minSampleShading	   = .2f;

	VkPipelineColorBlendAttachmentState colorBlendAttachment {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
		| VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
		| VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending {};
	colorBlending.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable		= VK_FALSE;
	colorBlending.logicOp			= VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount	= 1;
	colorBlending.pAttachments		= &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT,
												  VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount =
		static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts	  = &_descriptorSetLayout;

	if (vkCreatePipelineLayout(device,
							   &pipelineLayoutInfo, nullptr, &_pipelineLayout)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout !");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil {};
	depthStencil.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable	   = VK_TRUE;
	depthStencil.depthWriteEnable	   = VK_TRUE;
	depthStencil.depthCompareOp		   = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds		   = 0.0f;
	depthStencil.maxDepthBounds		   = 1.0f;
	depthStencil.stencilTestEnable	   = VK_FALSE;
	depthStencil.front				   = {};
	depthStencil.back				   = {};

	VkGraphicsPipelineCreateInfo pipelineInfo {};
	pipelineInfo.sType		= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages	= shaderStages;
	pipelineInfo.pVertexInputState	 = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState		 = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState	 = &multisampling;
	pipelineInfo.pColorBlendState	 = &colorBlending;
	pipelineInfo.pDynamicState		 = &dynamicState;
	pipelineInfo.layout				 = _pipelineLayout;
	pipelineInfo.renderPass			 = renderPass;
	pipelineInfo.subpass			 = 0;
	pipelineInfo.basePipelineHandle	 = VK_NULL_HANDLE;
	pipelineInfo.pDepthStencilState	 = &depthStencil;

	if (vkCreateGraphicsPipelines(device,
								  VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
								  &_pipeline)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline !");
	}
}

void evan::Renderer::createDescriptorPool(VkDevice device, uint32_t materialCount)
{
    uint32_t descriptorCount = materialCount * MAX_FRAMES_IN_FLIGHT;

	std::array<VkDescriptorPoolSize, 2> poolSizes {};
	poolSizes[0].type			 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = descriptorCount;
	poolSizes[1].type			 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = descriptorCount;

	VkDescriptorPoolCreateInfo poolInfo {};
	poolInfo.sType		   = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes	   = poolSizes.data();
	poolInfo.maxSets	   = descriptorCount;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr,
							   &_descriptorPool)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

void evan::Renderer::createFrame(VkCommandPool commandPool, const ADeviceBackend &deviceBackend)
{
	_frames.emplace_back(commandPool, deviceBackend);
}

VkDescriptorPool evan::Renderer::getDescriptorPool() const
{
	return _descriptorPool;
}

const std::vector<VkBuffer>& evan::Renderer::getUniformBuffers() const
{
	std::vector<VkBuffer> uniformBuffers;
	for (const Frame& frame: _frames) {
		uniformBuffers.push_back(frame.getUniformBuffer());
	}
	return uniformBuffers;
}

VkDescriptorSetLayout evan::Renderer::getDescriptorSetLayout() const
{
	return _descriptorSetLayout;
}
