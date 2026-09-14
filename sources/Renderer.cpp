/*
** ETIB PROJECT, 2026
** evan
** File description:
** Renderer
*/

#include "evan/Renderer.hpp"

#include "evan/Frustum.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cstddef>
#include <cstdlib>

namespace
{
	/**
	 * @brief Per-frame draw statistics, emitted once per command buffer
	 * instead of per mesh.
	 */
	struct DrawStats {
		std::size_t totalMeshes = 0;
		std::size_t visibleMeshes = 0;
		std::size_t culledMeshes = 0;
		std::size_t drawCalls = 0;
		std::size_t instancedDraws = 0;
		std::size_t pipelineBinds = 0;
		std::size_t descriptorBinds = 0;
		std::size_t skippedMeshes = 0;
	};

	/**
	 * @brief A mesh resolved for command buffer recording.
	 *
	 * Built once per command buffer, sorted so consecutive draws share
	 * pipeline and descriptor state, then recorded with bind commands issued
	 * only when the bound state changes.
	 */
	struct DrawItem {
		uint32_t pipelineKey = 0;
		uint32_t materialID = 0;
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		VkBuffer vertexBuffer = VK_NULL_HANDLE;
		VkBuffer indexBuffer = VK_NULL_HANDLE;
		uint32_t indexCount = 0;
		glm::mat4 transform = glm::mat4(1.0f);
	};

	/**
	 * @brief Whether verbose per-mesh draw logging is enabled.
	 *
	 * Controlled by the EVAN_DEBUG_DRAW_LOG environment variable. Off by
	 * default; set to any non-empty, non-"0" value to re-enable per-mesh
	 * trace output and the aggregate draw statistics.
	 */
	bool isDrawLogEnabled()
	{
		static const bool enabled = [] {
			const char *value = std::getenv("EVAN_DEBUG_DRAW_LOG");
			return value != nullptr && value[0] != '\0' && value[0] != '0';
		}();
		return enabled;
	}

	/**
	 * @brief Whether frustum/distance culling is disabled.
	 *
	 * Controlled by the EVAN_DEBUG_DISABLE_CULLING environment variable. Off
	 * by default; set to any non-empty, non-"0" value to draw every mesh
	 * regardless of its bounds.
	 */
	bool isCullingDisabledEnv()
	{
		static const bool disabled = [] {
			const char *value = std::getenv("EVAN_DEBUG_DISABLE_CULLING");
			return value != nullptr && value[0] != '\0' && value[0] != '0';
		}();
		return disabled;
	}
}	 // namespace

evan::Renderer::Renderer(std::shared_ptr<DeviceContext> deviceContext,
						 VkRenderPass renderPass,
						 VkSampleCountFlagBits msaaSamples,
						 std::shared_ptr<RessourceManager> ressourceManager)
	: _ressourceManager(ressourceManager)
	, _deviceContext(deviceContext)
{
	this->getLogger().info() << "Initializing Renderer...";

	_cullingEnabled = !isCullingDisabledEnv();

	_ressourceManager->sync();
	this->_currentFrameIndex = 0;

	this->getLogger().info() << "Current frame index: " << _currentFrameIndex;

	this->createDescriptorSetLayout(
		deviceContext->getDeviceBackend()->getDevice());
	this->createGraphicsPipelines(
		deviceContext->getDeviceBackend()->getDevice(), renderPass,
		msaaSamples);
	this->createDescriptorPool(
		deviceContext->getDeviceBackend()->getDevice(),
		1000);	  // TODO: Change this with the AssetManager when it will be
				  // implemented
	this->getLogger().info() << "Renderer initialized successfully.";

	this->getLogger().info() << "Creating frames for rendering...";
	for (int frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++) {
		_frames.emplace_back(std::make_shared<Frame>(deviceContext));
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

evan::Error evan::Renderer::drawFrame(const DeviceContext &deviceContext,
									  ASwapchainContext &swapchainContext,
									  const Scene &scene)
{
	this->getLogger().info() << "Drawing frame...";

	auto device = deviceContext.getDeviceBackend()->getDevice();

	Error preprocessError =
		deviceContext.getDeviceBackend()->preprocessFrame(swapchainContext);
	if (preprocessError != Error::Ok) {
		this->getLogger().warning()
			<< "Preprocessing frame failed. Skipping frame rendering.";
		return preprocessError;
	}

	if (swapchainContext.needsSwapchainRecreation()) {
		this->getLogger().warning()
			<< "Swapchain marked for recreation (window resized). Recreating "
			   "swapchain.";
		swapchainContext.recreateSwapchain(deviceContext,
										   swapchainContext.getRenderPass());
		return Error::SwapchainOutOfDate;
	}

	auto &frame						 = *_frames[_currentFrameIndex];
	const ViewSet &viewSet			 = swapchainContext.getViewSet();
	const std::size_t swapchainCount = swapchainContext.getSwapchainCount();

	this->getLogger().info()
		<< "Rendering " << viewSet.size() << " view(s) across "
		<< swapchainCount << " swapchain(s).";

	const bool waitOnImageAvailable =
		swapchainContext.usesImageAvailableSemaphore();

	// 1. Wait for the previous frame using this slot to finish on the GPU.
	// This guarantees the command buffer, the uniform buffer and the
	// semaphores are free to be reused for this frame. The fence is only
	// reset right before the first queue submission, so that early returns
	// (e.g. swapchain recreation) leave it signaled and the next call does
	// not deadlock.
	this->getLogger().info() << "Waiting for in-flight fence...";
	VkResult fenceResult =
		vkWaitForFences(device, 1, &frame._inFlight, VK_TRUE, UINT64_MAX);
	if (fenceResult != VK_SUCCESS) {
		this->getLogger().error()
			<< "Failed to wait for in-flight fence. Aborting frame rendering.";
		return mapVkResult(fenceResult);
	}

	// 2. Acquire exactly one image per swapchain image set.
	std::vector<uint32_t> acquiredImage(swapchainCount, 0);
	for (std::size_t s = 0; s < swapchainCount; ++s) {
		this->getLogger().info()
			<< "Acquiring swapchain image for swapchain " << s << "...";
		VkResult result = swapchainContext.aquireImage(
			static_cast<uint32_t>(s), device, frame._imageAvailable[s],
			VK_NULL_HANDLE, acquiredImage[s]);

		swapchainContext.waitForImage(static_cast<uint32_t>(s));

		Error acquireError = mapVkResult(result);
		if (acquireError == Error::SwapchainOutOfDate
			|| acquireError == Error::Suboptimal) {
			this->getLogger().warning()
				<< "Swapchain " << s
				<< " is out of date. Recreating swapchain.";
			swapchainContext.recreateSwapchain(
				deviceContext, swapchainContext.getRenderPass());
			return acquireError;
		} else if (acquireError != Error::Ok) {
			this->getLogger().error()
				<< "Failed to acquire swapchain image for swapchain " << s
				<< ". Aborting frame rendering.";
			return acquireError;
		}
	}

	// 3. Render each view into the acquired image of its swapchain. Each view
	// owns a command buffer and a uniform buffer slot, so all views can be
	// recorded and submitted back-to-back without waiting for the GPU between
	// them. The in-flight fence is signaled only by the last submission: the
	// graphics queue executes submissions in order, so its completion implies
	// every earlier view has completed as well.
	std::vector<std::size_t> renderableViews;
	for (std::size_t v = 0; v < viewSet.size(); ++v) {
		const std::size_t s = viewSet[v].swapchainIndex;
		if (s >= swapchainCount) {
			this->getLogger().error()
				<< "View " << v << " references invalid swapchain index " << s
				<< ". Skipping view.";
			continue;
		}
		renderableViews.push_back(v);
	}

	if (renderableViews.size() > MAX_SWAPCHAINS) {
		this->getLogger().error()
			<< "Frame has " << renderableViews.size() << " views, but only "
			<< MAX_SWAPCHAINS
			<< " per-view resources are available. Aborting frame rendering.";
		return Error::RuntimeError;
	}

	if (!renderableViews.empty()) {
		vkResetFences(device, 1, &frame._inFlight);
	}

	for (std::size_t i = 0; i < renderableViews.size(); ++i) {
		const std::size_t v		  = renderableViews[i];
		const ViewSet::View &view = viewSet[v];
		const std::size_t s		  = view.swapchainIndex;

		auto &imageSet = *swapchainContext._swapchainImages[s];

		this->updateUniformBuffer(scene, view.view, i);
		frame.resetCommandBuffer(i);
		this->recordCommandBuffer(swapchainContext.getRenderPass(),
								  imageSet.getFramebuffer(acquiredImage[s]),
								  imageSet.getExtent(), scene, i, view.view);

		VkPipelineStageFlags waitStages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};
		VkSubmitInfo submitInfo {};
		submitInfo.sType			  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = waitOnImageAvailable ? 1u : 0u;
		submitInfo.pWaitSemaphores =
			waitOnImageAvailable ? &frame._imageAvailable[s] : nullptr;
		submitInfo.pWaitDstStageMask	= waitStages;
		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &frame._commandBuffers[i];
		submitInfo.signalSemaphoreCount = waitOnImageAvailable ? 1u : 0u;
		submitInfo.pSignalSemaphores =
			waitOnImageAvailable ? &frame._renderFinished[s] : nullptr;

		const bool isLastView = (i + 1 == renderableViews.size());
		VkFence submitFence = isLastView ? frame._inFlight : VK_NULL_HANDLE;

		VkResult submitResult = vkQueueSubmit(deviceContext.getGraphicsQueue(),
											  1, &submitInfo, submitFence);
		if (submitResult != VK_SUCCESS) {
			this->getLogger().error()
				<< "Failed to submit draw command buffer for view " << v
				<< ". Aborting frame rendering.";
			return mapVkResult(submitResult);
		}
	}

	// OpenXR does not hand a Vulkan semaphore to the runtime, so the host must
	// guarantee the render completed before the swapchain images are released
	// and xrEndFrame hands them to the compositor. This is now a single
	// end-of-frame synchronization point covering all views, instead of one
	// wait between the eyes plus another before release.
	if (!waitOnImageAvailable && !renderableViews.empty()) {
		VkResult renderWaitResult =
			vkWaitForFences(device, 1, &frame._inFlight, VK_TRUE, UINT64_MAX);
		if (renderWaitResult != VK_SUCCESS) {
			this->getLogger().error()
				<< "Failed to wait for render completion. Aborting frame "
				   "rendering.";
			return mapVkResult(renderWaitResult);
		}
	}

	// 4. Present each swapchain once, with the image acquired for it.
	for (std::size_t s = 0; s < swapchainCount; ++s) {
		VkPresentInfoKHR presentInfo {};
		presentInfo.sType			   = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = waitOnImageAvailable ? 1u : 0u;
		presentInfo.pWaitSemaphores =
			waitOnImageAvailable ? &frame._renderFinished[s] : nullptr;

		swapchainContext._swapchainImages[s]->fillPresentInfo(presentInfo);
		presentInfo.pImageIndices = &acquiredImage[s];

		Error presentError = deviceContext.getDeviceBackend()->processFrame(
			presentInfo, *swapchainContext._swapchainImages[s]);
		if (presentError == Error::SwapchainOutOfDate
			|| presentError == Error::Suboptimal) {
			this->getLogger().warning()
				<< "Swapchain " << s
				<< " is out of date. Recreating swapchain.";
			swapchainContext.recreateSwapchain(
				deviceContext, swapchainContext.getRenderPass());
			return presentError;
		} else if (presentError != Error::Ok) {
			this->getLogger().error()
				<< "Failed to present swapchain " << s << ".";
			return presentError;
		}
	}

	this->getLogger().info() << "Frame drawn successfully.";

	_currentFrameIndex = (_currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	this->getLogger().info() << "Next frame index: " << _currentFrameIndex;

	this->getLogger().info() << "Post-processing frame...";
	Error postprocessError =
		deviceContext.getDeviceBackend()->postprocessFrame(swapchainContext);
	if (postprocessError != Error::Ok) {
		return postprocessError;
	}

	return Error::Ok;
}

void evan::Renderer::createFrame(std::shared_ptr<DeviceContext> deviceContext)
{
	this->getLogger().info() << "Creating frame with device context...";
	_frames.emplace_back(std::make_shared<Frame>(deviceContext));
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

	// UBO — dynamic so one buffer with per-view slots can be bound with a
	// per-view dynamic offset.
	bindings.push_back(
		{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1,
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
		auto bindingDescription = GPUVertex::getBindingDescription();
		auto instanceBinding = GPUVertex::getInstanceBindingDescription();
		auto attributeDescriptions = GPUVertex::getAttributeDescriptions();
		auto instanceAttributes = GPUVertex::getInstanceAttributeDescriptions();

		std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
			bindingDescription, instanceBinding
		};
		std::vector<VkVertexInputAttributeDescription> allAttributes;
		allAttributes.reserve(attributeDescriptions.size()
							  + instanceAttributes.size());
		allAttributes.insert(allAttributes.end(),
							 attributeDescriptions.begin(),
							 attributeDescriptions.end());
		allAttributes.insert(allAttributes.end(), instanceAttributes.begin(),
							 instanceAttributes.end());

		vertexInputInfo.vertexBindingDescriptionCount =
			static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.vertexAttributeDescriptionCount =
			static_cast<uint32_t>(allAttributes.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputInfo.pVertexAttributeDescriptions = allAttributes.data();

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
		// The GPUShader modules are owned by the RessourceManager. They are
		// released when the manager replaces or drops the wrapper, so do not
		// destroy them here or RessourceManager would double-free them.
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
	poolSizes[0].type			 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
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
		<< "Resetting command buffers for current frame index: "
		<< _currentFrameIndex;
	for (std::size_t viewSlot = 0; viewSlot < MAX_SWAPCHAINS; ++viewSlot) {
		_frames[_currentFrameIndex]->resetCommandBuffer(viewSlot);
	}
}

void evan::Renderer::updateUniformBuffer(const Scene &scene,
										 const utility::graphic::ViewF &view,
										 std::size_t viewSlot)
{
	this->getLogger().info()
		<< "Updating uniform buffer for current frame index: "
		<< _currentFrameIndex << ", view slot: " << viewSlot;

	Frame::UniformBufferObject ubo {};
	ubo.model = glm::mat4(1.0f);
	ubo.view  = view.toViewMatrix();
	ubo.proj  = view.getProjectionMatrix();

	memcpy(_frames[_currentFrameIndex]->getUniformBufferMapped(viewSlot),
		   &ubo, sizeof(ubo));
	this->getLogger().info() << "Uniform buffer updated successfully.";
}

void evan::Renderer::recordCommandBuffer(VkRenderPass renderPass,
										 VkFramebuffer swapChainFramebuffer,
										 VkExtent2D swapChainExtent,
										 const Scene &scene,
										 std::size_t viewSlot,
										 const utility::graphic::ViewF &view)
{
	if (isDrawLogEnabled()) {
		this->getLogger().debug()
			<< "Recording command buffer for current frame index: "
			<< _currentFrameIndex << ", view slot: " << viewSlot;
	}

	_ressourceManager->sync();

	auto &frame = *_frames[_currentFrameIndex];
	auto commandBuffer = frame.getCommandBuffer(viewSlot);

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

	if (isDrawLogEnabled()) {
		this->getLogger().debug() << "Beginning render pass...";
	}
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

	if (isDrawLogEnabled()) {
		this->getLogger().debug()
			<< "Viewport set to cover entire swapchain extent: "
			<< swapChainExtent.width << "x" << swapChainExtent.height;
	}

	VkRect2D scissor {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	if (isDrawLogEnabled()) {
		this->getLogger().debug()
			<< "Scissor set to cover entire swapchain extent: "
			<< swapChainExtent.width << "x" << swapChainExtent.height;
	}

	const auto &meshes = scene.getMeshes();

	DrawStats stats {};
	stats.totalMeshes = meshes.size();

	const glm::mat4 projection = view.getProjectionMatrix();
	const glm::mat4 viewMatrix = view.toViewMatrix();
	const Frustum frustum =
		Frustum::fromViewProjection(projection * viewMatrix);

	const auto cameraPosition = view.getPose().getPosition();
	const glm::vec3 cameraPos(cameraPosition.x, cameraPosition.y,
							  cameraPosition.z);

	const uint32_t dynamicOffset = static_cast<uint32_t>(
		viewSlot * frame.getUniformBufferAlignedSize());

	// Build the draw list: cull, resolve each mesh's pipeline, material and
	// descriptor state once, then sort so consecutive draws share GPU state.
	std::vector<DrawItem> drawList;
	drawList.reserve(meshes.size());

	for (const auto &mesh: meshes) {
		if (_cullingEnabled) {
			const auto &bounds = mesh->getBounds();
			if (!bounds.isEmpty()) {
				if (_maxDrawDistance > 0.0f) {
					const glm::vec3 center(bounds.center().x,
										   bounds.center().y,
										   bounds.center().z);
					if (glm::distance(cameraPos, center)
							- bounds.radius()
						> _maxDrawDistance) {
						++stats.culledMeshes;
						continue;
					}
				}
				if (!frustum.intersects(bounds)) {
					++stats.culledMeshes;
					continue;
				}
			}
		}
		++stats.visibleMeshes;

		if (isDrawLogEnabled()) {
			this->getLogger().debug()
				<< "Processing mesh with material ID: "
				<< mesh->getMaterialID();
		}
		const auto materialID = mesh->getMaterialID();
		const auto material = _ressourceManager->getMaterial(materialID);

		if (!material) {
			++stats.skippedMeshes;
			if (isDrawLogEnabled()) {
				this->getLogger().debug()
					<< "Material with ID " << materialID
					<< " not found! Skipping mesh.";
			}
			continue;
		}

		const auto pipelineKey = material->getShaderID();
		const auto pipelineIt = _pipelines.find(pipelineKey);
		if (pipelineIt == _pipelines.end()) {
			++stats.skippedMeshes;
			if (isDrawLogEnabled()) {
				this->getLogger().debug()
					<< "No pipeline found for shader ID: "
					<< pipelineKey << ". Skipping mesh.";
			}
			continue;
		}
		const auto layoutIt = _pipelineLayouts.find(pipelineKey);
		if (layoutIt == _pipelineLayouts.end()) {
			++stats.skippedMeshes;
			if (isDrawLogEnabled()) {
				this->getLogger().debug()
					<< "No pipeline layout found for shader ID: "
					<< pipelineKey << ". Skipping mesh.";
			}
			continue;
		}

		const auto vertexBuffer = mesh->getVertexBuffer();
		if (vertexBuffer == VK_NULL_HANDLE) {
			++stats.skippedMeshes;
			if (isDrawLogEnabled()) {
				this->getLogger().debug()
					<< "Vertex buffer is null for mesh with material ID: "
					<< materialID << ". Skipping mesh.";
			}
			continue;
		}

		const auto indexBuffer = mesh->getIndexBuffer();
		if (indexBuffer == VK_NULL_HANDLE) {
			++stats.skippedMeshes;
			if (isDrawLogEnabled()) {
				this->getLogger().debug()
					<< "Index buffer is null for mesh with material ID: "
					<< materialID << ". Skipping mesh.";
			}
			continue;
		}

		const auto &descriptorSets = material->getDescriptorSets();
		if (_currentFrameIndex >= descriptorSets.size()) {
			++stats.skippedMeshes;
			if (isDrawLogEnabled()) {
				this->getLogger().debug()
					<< "No descriptor set for material ID " << materialID
					<< " at frame index " << _currentFrameIndex
					<< ". Skipping mesh.";
			}
			continue;
		}

		drawList.push_back(
			{ pipelineKey, materialID, pipelineIt->second, layoutIt->second,
			  descriptorSets[_currentFrameIndex], vertexBuffer, indexBuffer,
			  mesh->getIndexCount(), mesh->getTransform() });
	}

	std::sort(drawList.begin(), drawList.end(),
			  [](const DrawItem &lhs, const DrawItem &rhs) {
				  if (lhs.pipelineKey != rhs.pipelineKey) {
					  return lhs.pipelineKey < rhs.pipelineKey;
				  }
				  if (lhs.materialID != rhs.materialID) {
					  return lhs.materialID < rhs.materialID;
				  }
				  if (lhs.vertexBuffer != rhs.vertexBuffer) {
					  return lhs.vertexBuffer < rhs.vertexBuffer;
				  }
				  return lhs.indexBuffer < rhs.indexBuffer;
			  });

	// Record the sorted draw list, re-binding state only when it changes.
	VkPipeline boundPipeline = VK_NULL_HANDLE;
	VkPipelineLayout boundPipelineLayout = VK_NULL_HANDLE;
	VkDescriptorSet boundDescriptorSet = VK_NULL_HANDLE;
	VkBuffer boundVertexBuffer = VK_NULL_HANDLE;
	VkBuffer boundIndexBuffer = VK_NULL_HANDLE;
	VkBuffer boundInstanceBuffer = VK_NULL_HANDLE;
	uint32_t boundDynamicOffset = 0;
	bool hasBoundState = false;
	bool hasBoundInstanceBuffer = false;

	const VkDeviceSize instanceBufferOffset =
		viewSlot * frame.getInstanceBufferAlignedSize();
	GPUInstance *instanceData =
		_instancingEnabled
			? static_cast<GPUInstance *>(
				  frame.getInstanceBufferMapped(viewSlot))
			: nullptr;
	uint32_t instanceCursor = 0;

	const VkDeviceSize indirectBufferOffset =
		viewSlot * frame.getIndirectBufferAlignedSize();
	VkDrawIndexedIndirectCommand *indirectData =
		_indirectDrawingEnabled
			? static_cast<VkDrawIndexedIndirectCommand *>(
				  frame.getIndirectBufferMapped(viewSlot))
			: nullptr;
	uint32_t indirectCursor = 0;
	uint32_t indirectPendingBase = 0;

	const auto flushIndirect = [&]() {
		if (!_indirectDrawingEnabled
			|| indirectCursor == indirectPendingBase) {
			return;
		}
		vkCmdDrawIndexedIndirect(
			commandBuffer, frame.getIndirectBuffer(),
			indirectBufferOffset
				+ indirectPendingBase
					* sizeof(VkDrawIndexedIndirectCommand),
			indirectCursor - indirectPendingBase,
			static_cast<uint32_t>(sizeof(VkDrawIndexedIndirectCommand)));
		indirectPendingBase = indirectCursor;
	};

	const auto emitDraw = [&](uint32_t indexCount, uint32_t instanceCount,
							  uint32_t firstInstance) {
		if (_indirectDrawingEnabled
			&& indirectCursor
				< static_cast<uint32_t>(MAX_INDIRECT_COMMANDS_PER_VIEW)) {
			indirectData[indirectCursor++] = {
				indexCount, instanceCount, 0, 0, firstInstance
			};
		} else {
			vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0,
							 firstInstance);
		}
		++stats.drawCalls;
	};

	for (std::size_t i = 0; i < drawList.size();) {
		// Merge consecutive items sharing pipeline, material and geometry.
		std::size_t runEnd = i + 1;
		while (runEnd < drawList.size()
			   && drawList[runEnd].pipelineKey == drawList[i].pipelineKey
			   && drawList[runEnd].materialID == drawList[i].materialID
			   && drawList[runEnd].vertexBuffer == drawList[i].vertexBuffer
			   && drawList[runEnd].indexBuffer == drawList[i].indexBuffer) {
			++runEnd;
		}
		const std::size_t runSize = runEnd - i;
		const DrawItem &item = drawList[i];

		const bool stateChanged =
			!hasBoundState || boundPipeline != item.pipeline
			|| boundVertexBuffer != item.vertexBuffer
			|| boundIndexBuffer != item.indexBuffer
			|| boundDescriptorSet != item.descriptorSet
			|| boundPipelineLayout != item.pipelineLayout
			|| boundDynamicOffset != dynamicOffset
			|| (_instancingEnabled && !hasBoundInstanceBuffer);

		if (stateChanged) {
			flushIndirect();
		}

		if (!hasBoundState || boundPipeline != item.pipeline) {
			if (isDrawLogEnabled()) {
				this->getLogger().debug()
					<< "Binding pipeline for shader ID: "
					<< item.pipelineKey;
			}
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
							  item.pipeline);
			boundPipeline = item.pipeline;
			++stats.pipelineBinds;

			glm::vec4 color { 1.f, 1.f, 1.f, 1.f };
			vkCmdPushConstants(
				commandBuffer, item.pipelineLayout,
				VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::vec4), &color);
		}

		if (!hasBoundState || boundVertexBuffer != item.vertexBuffer) {
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1,
								   &item.vertexBuffer, offsets);
			boundVertexBuffer = item.vertexBuffer;
			if (isDrawLogEnabled()) {
				this->getLogger().debug() << "Binding vertex buffer for ->..";
			}
		}

		if (!hasBoundState || boundIndexBuffer != item.indexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, item.indexBuffer, 0,
								 VK_INDEX_TYPE_UINT32);
			boundIndexBuffer = item.indexBuffer;
		}

		if (!hasBoundState || boundDescriptorSet != item.descriptorSet
			|| boundPipelineLayout != item.pipelineLayout
			|| boundDynamicOffset != dynamicOffset) {
			if (isDrawLogEnabled()) {
				this->getLogger().debug()
					<< "Binding descriptor set for material ID: "
					<< item.materialID;
			}
			vkCmdBindDescriptorSets(
				commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				item.pipelineLayout, 0, 1, &item.descriptorSet, 1,
				&dynamicOffset);
			boundDescriptorSet = item.descriptorSet;
			boundPipelineLayout = item.pipelineLayout;
			boundDynamicOffset = dynamicOffset;
			++stats.descriptorBinds;
		}

		if (_instancingEnabled
			&& (!hasBoundInstanceBuffer
				|| boundInstanceBuffer != frame.getInstanceBuffer())) {
			VkBuffer instanceBuffer = frame.getInstanceBuffer();
			VkDeviceSize instanceOffsets[] = { instanceBufferOffset };
			vkCmdBindVertexBuffers(commandBuffer, 1, 1,
								   &instanceBuffer, instanceOffsets);
			boundInstanceBuffer = instanceBuffer;
			hasBoundInstanceBuffer = true;
		}

		if (_instancingEnabled) {
			if (instanceCursor + runSize
				> static_cast<std::size_t>(MAX_INSTANCES_PER_VIEW)) {
				this->getLogger().warning()
					<< "Instance buffer capacity exceeded ("
					<< MAX_INSTANCES_PER_VIEW << " per view). Skipping "
					<< runSize << " meshes.";
				stats.skippedMeshes += runSize;
				i = runEnd;
				hasBoundState = true;
				continue;
			}
			for (std::size_t j = 0; j < runSize; ++j) {
				instanceData[instanceCursor + j].model =
					drawList[i + j].transform;
			}
			if (isDrawLogEnabled()) {
				this->getLogger().debug()
					<< "Drawing indexed mesh with index count: "
					<< item.indexCount << " and instance count: "
					<< runSize;
			}
			emitDraw(item.indexCount, static_cast<uint32_t>(runSize),
					 instanceCursor);
			if (runSize > 1) {
				++stats.instancedDraws;
			}
			instanceCursor += static_cast<uint32_t>(runSize);
		} else {
			for (std::size_t j = i; j < runEnd; ++j) {
				if (isDrawLogEnabled()) {
					this->getLogger().debug()
						<< "Drawing indexed mesh with index count: "
						<< drawList[j].indexCount;
				}
				emitDraw(drawList[j].indexCount, 1, 0);
			}
		}

		i = runEnd;
		hasBoundState = true;
	}

	flushIndirect();

	if (isDrawLogEnabled()) {
		this->getLogger().debug()
			<< "Draw stats: totalMeshes=" << stats.totalMeshes
			<< " visibleMeshes=" << stats.visibleMeshes
			<< " culledMeshes=" << stats.culledMeshes
			<< " drawCalls=" << stats.drawCalls
			<< " instancedDraws=" << stats.instancedDraws
			<< " pipelineBinds=" << stats.pipelineBinds
			<< " descriptorBinds=" << stats.descriptorBinds
			<< " skippedMeshes=" << stats.skippedMeshes;
		this->getLogger().debug() << "All meshes processed. Ending render "
									 "pass...";
	}

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		this->getLogger().error() << "Failed to record command buffer!";
		return;
	}
}
