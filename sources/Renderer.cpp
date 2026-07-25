/*
** ETIB PROJECT, 2026
** evan
** File description:
** Renderer
*/

#include "evan/Renderer.hpp"

#include <glm/gtc/matrix_transform.hpp>

evan::Renderer::Renderer(DeviceContext &deviceContext, VkRenderPass renderPass,
						 VkSampleCountFlagBits msaaSamples,
						 std::shared_ptr<RessourceManager> ressourceManager)
	: _ressourceManager(ressourceManager)
{
	this->getLogger().info() << "Initializing Renderer...";

	_ressourceManager->sync();
	this->_currentFrameIndex = 0;

	this->getLogger().info() << "Current frame index: " << _currentFrameIndex;

	this->createDescriptorSetLayout(deviceContext.getDeviceBackend()->_device);
	this->createGraphicsPipelines(deviceContext.getDeviceBackend()->_device,
								  renderPass, msaaSamples);
	this->createDescriptorPool(
		deviceContext.getDeviceBackend()->_device,
		1000);	  // TODO: Change this with the AssetManager when it will be
				  // implemented
	this->getLogger().info() << "Renderer initialized successfully.";

	this->getLogger().info() << "Creating frames for rendering...";
	for (int frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++) {
		_frames.emplace_back(std::make_shared<Frame>(
			deviceContext.getCommandPool(), *deviceContext.getDeviceBackend()));
	}
	this->getLogger().info() << "Frames created successfully.";
}

evan::Renderer::~Renderer()
{
	this->getLogger().info() << "Destroying Renderer...";
}

void evan::Renderer::destroy(VkDevice device)
{
	this->getLogger().info() << "Destroying Renderer resources...";

	this->getLogger().info() << "Destroying descriptor pool...";
	vkDestroyDescriptorPool(device, _descriptorPool, nullptr);

	this->getLogger().info() << "Destroying descriptor set layout...";
	vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);

	this->getLogger().info() << "Destroying graphics pipelines...";
	for (const auto &[id, pipeline]: _pipelines) {
		this->getLogger().info() << "Destroying pipeline: " << id;
		vkDestroyPipeline(device, pipeline, nullptr);
	}
	for (const auto &[id, pipelineLayout]: _pipelineLayouts) {
		this->getLogger().info() << "Destroying pipeline layout: " << id;
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	}

	this->getLogger().info() << "Destroying frames...";
	for (const auto &frame: _frames) {
		frame->destroy(device);
	}
}

void evan::Renderer::drawFrame(const DeviceContext &deviceContext,
							   ASwapchainContext &swapchainContext,
							   const Scene &scene)
{
	this->getLogger().info() << "Drawing frame...";

	if (!deviceContext.getDeviceBackend()->preprocessFrame(swapchainContext)) {
		this->getLogger().warning()
			<< "Preprocessing frame failed. Skipping frame rendering.";
		return;
	}

	this->getLogger().info()
		<< "Acquiring swapchain image and recording command buffer...";
	for (int i = 0; i < swapchainContext._swapchainImages.size(); i++) {
		this->getLogger().info() << "Processing swapchain image index: " << i;

		this->getLogger().info() << "Waiting for in-flight fence...";
		vkWaitForFences(deviceContext.getDeviceBackend()->_device, 1,
						&_frames[_currentFrameIndex]->_inFlight, VK_TRUE,
						UINT64_MAX);

		uint32_t imageIndex;
		this->getLogger().info() << "Aquiring swapchain image...";
		auto result = swapchainContext.aquireImage(
			i, deviceContext.getDeviceBackend()->_device,
			_frames[_currentFrameIndex]->_image, VK_NULL_HANDLE, imageIndex);

		swapchainContext.waitForImage(i);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			swapchainContext.recreateSwapchain(
				deviceContext, swapchainContext.getRenderPass());
		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			this->getLogger().error() << "Failed to acquire swap chain image! "
										 "Skipping frame rendering.";
			this->getLogger().error() << "Vulkan error code: " << result;
			this->getLogger().warning()
				<< "Aborting frame rendering due to image acquisition failure.";
			return;
		}

		this->updateUniformBuffer(scene, swapchainContext, i);

		this->getLogger().info() << "Resetting in-flight fence...";
		vkResetFences(deviceContext.getDeviceBackend()->_device, 1,
					  &_frames[_currentFrameIndex]->_inFlight);

		this->resetCommandBuffers();

		auto swapchainExtent =
			swapchainContext._swapchainImages[i]->getExtent();

		this->recordCommandBuffer(
			swapchainContext.getRenderPass(),
			swapchainContext._swapchainImages[i]->getFramebuffer(imageIndex),
			swapchainExtent, scene);

		VkPipelineStageFlags waitStages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};
		VkSemaphore signalSemaphores[] = {
			_frames[_currentFrameIndex]->_render
		};
		VkSubmitInfo submitInfo {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		// submitInfo.waitSemaphoreCount = 1;
		// submitInfo.pWaitSemaphores = &_frames[_currentFrameIndex]._image;
		submitInfo.pWaitDstStageMask  = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers =
			&_frames[_currentFrameIndex]->_commandBuffer;
		// submitInfo.signalSemaphoreCount = 1;
		// submitInfo.pSignalSemaphores = signalSemaphores;

		this->getLogger().info()
			<< "Submitting command buffer to graphics queue...";
		this->getLogger().info()
			<< "Current frame index: " << _currentFrameIndex;

		if (vkQueueSubmit(deviceContext.getGraphicsQueue(), 1, &submitInfo,
						  _frames[_currentFrameIndex]->_inFlight)
			!= VK_SUCCESS) {
			this->getLogger().error()
				<< "Failed to submit draw command buffer! "
				   "Skipping frame rendering.";
			return;
		}

		VkPresentInfoKHR presentInfo {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		// presentInfo.waitSemaphoreCount = 1;
		// presentInfo.pWaitSemaphores = signalSemaphores;
		swapchainContext._swapchainImages[i]->fillPresentInfo(presentInfo);
		presentInfo.pImageIndices = &imageIndex;

		if (!deviceContext.getDeviceBackend()->processFrame(
				presentInfo, *swapchainContext._swapchainImages[i].get())) {
			this->getLogger().error() << "Failed to present swap chain image! "
										 "Skipping frame rendering.";
			// The swapchain is out of date (e.g. the window was resized) and
			// must be recreated.
			continue;
		}
	}
	this->getLogger().info() << "Frame drawn successfully.";

	_currentFrameIndex = (_currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	this->getLogger().info() << "Next frame index: " << _currentFrameIndex;

	this->getLogger().info() << "Post-processing frame...";
	deviceContext.getDeviceBackend()->postprocessFrame(swapchainContext);
}

void evan::Renderer::createFrame(VkCommandPool commandPool,
								 const ADeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Creating frame with command pool...";
	_frames.emplace_back(std::make_shared<Frame>(commandPool, deviceBackend));
}

/////////////
// Getters //
/////////////

VkDescriptorPool evan::Renderer::getDescriptorPool() const
{
	return _descriptorPool;
}

const std::vector<VkBuffer> evan::Renderer::getUniformBuffers() const
{
	std::vector<VkBuffer> uniformBuffers;
	for (const auto &frame: _frames) {
		uniformBuffers.push_back(frame->getUniformBuffer());
	}
	return uniformBuffers;
}

VkDescriptorSetLayout evan::Renderer::getDescriptorSetLayout() const
{
	return _descriptorSetLayout;
}

/////////////////////
// Private methods //
/////////////////////

void evan::Renderer::createDescriptorSetLayout(VkDevice device)
{
	this->getLogger().info() << "Creating descriptor set layout...";

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	// UBO
	bindings.push_back(
		{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
		  VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr });
	this->getLogger().info()
		<< "Added uniform buffer binding to descriptor set layout.";

	// Textures -> TODO: Adapt this according to all the textures that will be
	// supported by the engine (e.g. normal map, metallic roughness map, etc.)
	bindings.push_back({ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
						 VK_SHADER_STAGE_FRAGMENT_BIT });	 // Albedo
	bindings.push_back({ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
						 VK_SHADER_STAGE_FRAGMENT_BIT });	 // Normal
	bindings.push_back(
		{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
		  VK_SHADER_STAGE_FRAGMENT_BIT });	  // MetallicRoughness

	VkDescriptorSetLayoutBinding fontAtlasBinding {};
	fontAtlasBinding.binding		 = 4;
	fontAtlasBinding.descriptorCount = 1;
	fontAtlasBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	fontAtlasBinding.stageFlags		= VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(fontAtlasBinding);

	this->getLogger().info()
		<< "Added texture bindings to descriptor set layout.";

	VkDescriptorSetLayoutCreateInfo layoutInfo {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings	= bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
									&_descriptorSetLayout)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to create descriptor set layout!";
		return;
	}
	this->getLogger().info() << "Descriptor set layout created successfully.";
}

void evan::Renderer::createGraphicsPipelines(VkDevice device,
											 VkRenderPass renderPass,
											 VkSampleCountFlagBits msaaSamples)
{
	this->getLogger().info() << "Creating graphics pipelines...";

	this->getLogger().info() << "Iterating over shaders to create pipelines...";
	for (const auto &[id, shader]: _ressourceManager->getShaders()) {
		this->getLogger().info() << "Creating pipeline for shader: " << id;

		VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
		vertShaderStageInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = shader->getVertexShaderModule();
		vertShaderStageInfo.pName  = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
		fragShaderStageInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = shader->getFragmentShaderModule();
		fragShaderStageInfo.pName  = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {
			vertShaderStageInfo, fragShaderStageInfo
		};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
		vertexInputInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		this->getLogger().info()
			<< "Getting vertex input descriptions for shader: " << id;
		auto bindingDescription	   = GPUVertex::getBindingDescription();
		auto attributeDescriptions = GPUVertex::getAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount =
			static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions =
			attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
		inputAssembly.sType =
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState {};
		viewportState.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
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

#ifdef DEV
		rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
#endif

		VkPipelineMultisampleStateCreateInfo multisampling {};
		multisampling.sType =
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable  = VK_FALSE;
		multisampling.rasterizationSamples = msaaSamples;
		multisampling.minSampleShading	   = .2f;

		VkPipelineColorBlendAttachmentState colorBlendAttachment {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
			| VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
			| VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable		 = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor =
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp		 = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor =
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

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

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_CULL_MODE
		};
		VkPipelineDynamicStateCreateInfo dynamicState {};
		dynamicState.sType =
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount =
			static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPushConstantRange pushConstantRange {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset	 = 0;
		pushConstantRange.size		 = sizeof(glm::vec4);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
		pipelineLayoutInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts	  = &_descriptorSetLayout;

		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges	  = &pushConstantRange;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
								   &_pipelineLayouts[id])
			!= VK_SUCCESS) {
			this->getLogger().error() << "Failed to create pipeline layout !";
			return;
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
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount			 = 2;
		pipelineInfo.pStages			 = shaderStages;
		pipelineInfo.pVertexInputState	 = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState		 = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState	 = &multisampling;
		pipelineInfo.pColorBlendState	 = &colorBlending;
		pipelineInfo.pDynamicState		 = &dynamicState;
		pipelineInfo.layout				 = _pipelineLayouts[id];
		pipelineInfo.renderPass			 = renderPass;
		pipelineInfo.subpass			 = 0;
		pipelineInfo.basePipelineHandle	 = VK_NULL_HANDLE;
		pipelineInfo.pDepthStencilState	 = &depthStencil;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
									  nullptr, &_pipelines[id])
			!= VK_SUCCESS) {
			this->getLogger().error() << "Failed to create graphics pipeline !";
			return;
		}
		shader->destroy();
	}
	this->getLogger().info() << "Graphics pipelines created successfully.";
}

void evan::Renderer::createDescriptorPool(VkDevice device,
										  uint32_t materialCount)
{
	this->getLogger().info() << "Creating descriptor pool...";

	uint32_t descriptorCount = materialCount * MAX_FRAMES_IN_FLIGHT;

	this->getLogger().info()
		<< "Descriptor count calculated: " << descriptorCount;

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

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to create descriptor pool!";
		return;
	}
	this->getLogger().info() << "Descriptor pool created successfully.";
}

void evan::Renderer::resetCommandBuffers()
{
	this->getLogger().info()
		<< "Resetting command buffer for current frame index: "
		<< _currentFrameIndex;
	_frames[_currentFrameIndex]->resetCommandBuffer();
}

void evan::Renderer::updateUniformBuffer(const Scene &scene,
										 ASwapchainContext &swapchainContext,
										 int currentIndex)
{
	this->getLogger().info()
		<< "Updating uniform buffer for current frame index: "
		<< _currentFrameIndex << " and swapchain image index: " << currentIndex;

	Frame::UniformBufferObject ubo {};
	ubo.model = glm::mat4(1.0f);
	ubo.view  = swapchainContext.getView(currentIndex).toViewMatrix();
	ubo.proj  = swapchainContext.getProjection(currentIndex);

	memcpy(_frames[_currentFrameIndex]->_uniformBufferMapped, &ubo,
		   sizeof(ubo));
	this->getLogger().info() << "Uniform buffer updated successfully.";
}

void evan::Renderer::recordCommandBuffer(VkRenderPass renderPass,
										 VkFramebuffer swapChainFramebuffer,
										 VkExtent2D swapChainExtent,
										 const Scene &scene)
{
	this->getLogger().info()
		<< "Recording command buffer for current frame index: "
		<< _currentFrameIndex;

	_ressourceManager->sync();

	auto commandBuffer = _frames[_currentFrameIndex]->_commandBuffer;

	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		this->getLogger().error()
			<< "Failed to begin recording command buffer!";
		return;
	}

	VkRenderPassBeginInfo renderPassInfo {};
	renderPassInfo.sType			 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass		 = renderPass;
	renderPassInfo.framebuffer		 = swapChainFramebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChainExtent;

	std::array<VkClearValue, 2> clearValues {};
	clearValues[0].color		= { { 0.184313729f, 0.309803933f, 0.309803933f,
									  1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues	   = clearValues.data();

	this->getLogger().info() << "Beginning render pass...";
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
						 VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport {};
	viewport.x		  = 0.0f;
	viewport.y		  = 0.0f;
	viewport.width	  = (float)swapChainExtent.width;
	viewport.height	  = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	this->getLogger().info()
		<< "Viewport set to cover entire swapchain extent: "
		<< swapChainExtent.width << "x" << swapChainExtent.height;

	VkRect2D scissor {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	this->getLogger().info()
		<< "Scissor set to cover entire swapchain extent: "
		<< swapChainExtent.width << "x" << swapChainExtent.height;

	std::map<uint32_t, bool> materialBound;

	this->getLogger().info()
		<< "Iterating over meshes in the scene to record draw commands...";

	for (const auto &mesh: scene.getMeshes()) {
		this->getLogger().info()
			<< "Processing mesh with material ID: " << mesh->getMaterialID();
		auto materialID = mesh->getMaterialID();
		auto material	= _ressourceManager->getMaterial(materialID);

		if (!material) {
			this->getLogger().warning() << "Material with ID " << materialID
										<< " not found! Skipping mesh.";
			continue;
		}

		auto correspondingPipelineID = material->getShaderID();

		if (_pipelines.find(correspondingPipelineID) == _pipelines.end()) {
			this->getLogger().warning()
				<< "No pipeline found for shader ID: "
				<< correspondingPipelineID << ". Skipping mesh.";
			continue;
		}

		this->getLogger().info()
			<< "Binding pipeline for shader ID: " << correspondingPipelineID;

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
						  _pipelines[correspondingPipelineID]);

		VkDeviceSize offsets[] = { 0 };
		VkBuffer vertexBuffer  = mesh->getVertexBuffer();

		this->getLogger().info() << "Binding vertex buffer for ->..";

		if (vertexBuffer == VK_NULL_HANDLE) {
			continue;
		}

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);

		this->getLogger().info() << "Binding index buffer for mesh...";

		auto indexBuffer = mesh->getIndexBuffer();

		if (indexBuffer == VK_NULL_HANDLE) {
			this->getLogger().warning()
				<< "Index buffer is null for mesh with material ID: "
				<< mesh->getMaterialID() << ". Skipping mesh.";
			continue;
		}

		vkCmdBindIndexBuffer(commandBuffer, mesh->getIndexBuffer(), 0,
							 VK_INDEX_TYPE_UINT32);

		if (!materialBound[mesh->getMaterialID()]) {
			this->getLogger().info()
				<< "Binding descriptor set for material ID: "
				<< mesh->getMaterialID();
			materialBound[mesh->getMaterialID()] = true;
			vkCmdBindDescriptorSets(
				commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				_pipelineLayouts[correspondingPipelineID], 0, 1,
				&material->getDescriptorSets()[_currentFrameIndex], 0, nullptr);
		}

		glm::vec4 color { 1.f, 1.f, 1.f, 1.f };

		vkCmdPushConstants(
			commandBuffer, _pipelineLayouts[correspondingPipelineID],
			VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::vec4), &color);

		this->getLogger().info() << "Drawing indexed mesh with index count: "
								 << mesh->getIndexCount();

		vkCmdDrawIndexed(commandBuffer, mesh->getIndexCount(), 1, 0, 0, 0);
	}

	this->getLogger().info() << "All meshes processed. Ending render pass...";

	vkCmdEndRenderPass(commandBuffer);

	this->getLogger().info()
		<< "Render pass ended. Ending command buffer recording...";

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		this->getLogger().error() << "Failed to record command buffer!";
		return;
	}
}
