/*
** ETIB PROJECT, 2026
** evan
** File description:
** SwapchainContext
*/

#include "evan/ASwapchainContext.hpp"

void evan::ASwapchainContext::createRenderPass(
	const std::shared_ptr<ADeviceBackend> &deviceBackend,
	VkSampleCountFlagBits msaaSamples)
{
	this->getLogger().info() << "Creating render pass for swapchain context...";

	auto swapchainFormatCount = deviceBackend->countSwapchainFormats();
	auto swapchainFormats =
		deviceBackend->enumerateSwapchainFormats(swapchainFormatCount);
	auto swapchainFormat = selectSwapchainFormat(swapchainFormats);

	VkAttachmentDescription colorAttachment {};
	colorAttachment.format		   = swapchainFormat;
	colorAttachment.loadOp		   = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp		   = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.samples		   = msaaSamples;
	colorAttachment.finalLayout	   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	this->getLogger().info()
		<< "Selected swapchain format: " << swapchainFormat;

	VkAttachmentDescription depthAttachment {};
	depthAttachment.format =
		ASwapchainImage::findDepthFormat(deviceBackend->_physicalDevice);
	depthAttachment.loadOp		   = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp		   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.samples = msaaSamples;

	this->getLogger().info()
		<< "Selected depth format: " << depthAttachment.format;

	VkAttachmentReference colorAttachmentRef {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout	  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	this->getLogger().info()
		<< "Configured color attachment reference with attachment index: "
		<< colorAttachmentRef.attachment
		<< " and layout: " << colorAttachmentRef.layout;

	VkAttachmentReference depthAttachmentRef {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	this->getLogger().info()
		<< "Configured depth attachment reference with attachment index: "
		<< depthAttachmentRef.attachment
		<< " and layout: " << depthAttachmentRef.layout;

	VkAttachmentDescription colorAttachmentResolve {};
	colorAttachmentResolve.format		  = swapchainFormat;
	colorAttachmentResolve.samples		  = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp		  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp		  = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout	  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	this->getLogger().info()
		<< "Configured color attachment resolve with format: "
		<< colorAttachmentResolve.format
		<< ", samples: " << colorAttachmentResolve.samples
		<< ", load operation: " << colorAttachmentResolve.loadOp
		<< ", store operation: " << colorAttachmentResolve.storeOp
		<< ", stencil load operation: " << colorAttachmentResolve.stencilLoadOp
		<< ", stencil store operation: "
		<< colorAttachmentResolve.stencilStoreOp
		<< ", initial layout: " << colorAttachmentResolve.initialLayout
		<< ", final layout: " << colorAttachmentResolve.finalLayout;

	VkAttachmentReference colorAttachmentResolveRef {};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	this->getLogger().info()
		<< "Configured color attachment resolve reference with attachment "
		   "index: "
		<< colorAttachmentResolveRef.attachment
		<< " and layout: " << colorAttachmentResolveRef.layout;

	VkSubpassDescription subpass {};
	subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount	= 1;
	subpass.pColorAttachments		= &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments		= &colorAttachmentResolveRef;

	this->getLogger().info()
		<< "Configured subpass with pipeline bind point: "
		<< subpass.pipelineBindPoint
		<< ", color attachment count: " << subpass.colorAttachmentCount
		<< ", depth-stencil attachment reference: "
		<< depthAttachmentRef.attachment << ", resolve attachment reference: "
		<< colorAttachmentResolveRef.attachment
		<< ", and color attachment reference: "
		<< colorAttachmentRef.attachment;

	VkSubpassDependency dependency {};
	dependency.srcSubpass	= VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass	= 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask	 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	this->getLogger().info()
		<< "Configured subpass dependency with source subpass: "
		<< dependency.srcSubpass
		<< ", destination subpass: " << dependency.dstSubpass
		<< ", source stage mask: " << dependency.srcStageMask
		<< ", source access mask: " << dependency.srcAccessMask
		<< ", destination stage mask: " << dependency.dstStageMask
		<< ", and destination access mask: " << dependency.dstAccessMask;

	this->getLogger().info()
		<< "Selected swapchain format: " << swapchainFormat;

	std::array<VkAttachmentDescription, 3> attachments = {
		colorAttachment, depthAttachment, colorAttachmentResolve
	};
	VkRenderPassCreateInfo renderPassInfo {};
	renderPassInfo.sType		   = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments	   = attachments.data();
	renderPassInfo.subpassCount	   = 1;
	renderPassInfo.pSubpasses	   = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies   = &dependency;

	this->getLogger().info()
		<< "Creating render pass with " << renderPassInfo.attachmentCount
		<< " attachments, " << renderPassInfo.subpassCount << " subpass, and "
		<< renderPassInfo.dependencyCount << " dependency.";

	if (vkCreateRenderPass(deviceBackend->_device, &renderPassInfo, nullptr,
						   &_renderPass)
		!= VK_SUCCESS) {
		this->getLogger().error()
			<< "Failed to create render pass for swapchain context.";
		return;
	}

	this->getLogger().info()
		<< "Successfully created render pass for swapchain context.";
}

VkRenderPass evan::ASwapchainContext::getRenderPass() const
{
	return _renderPass;
}

VkFormat evan::ASwapchainContext::selectSwapchainFormat(
	const std::vector<int64_t> &swapchainFormats)
{
	this->getLogger().info()
		<< "Selecting swapchain format from " << swapchainFormats.size()
		<< " available formats...";

	constexpr VkFormat kPreferredSwapchainFormats[] = {
		VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM
	};

	for (size_t i = 0; i < swapchainFormats.size(); ++i) {
		this->getLogger().info() << "Available swapchain format " << i << ": "
								 << swapchainFormats[i];
	}

	auto swapchainFormatIt =
		std::find_first_of(swapchainFormats.begin(), swapchainFormats.end(),
						   std::begin(kPreferredSwapchainFormats),
						   std::end(kPreferredSwapchainFormats));

	if (swapchainFormatIt == swapchainFormats.end()) {
		this->getLogger().info() << "No preferred swapchain format found, "
									"using first available format.";
		return static_cast<VkFormat>(swapchainFormats[0]);
	}

	this->getLogger().info()
		<< "Selected preferred swapchain format: " << *swapchainFormatIt;

	return static_cast<VkFormat>(*swapchainFormatIt);
}

VkFormat evan::ASwapchainContext::findSupportedFormat(
	VkPhysicalDevice physicalDevice, const std::vector<VkFormat> &candidates,
	VkImageTiling tiling, VkFormatFeatureFlags features)
{
	this->getLogger().info()
		<< "Finding supported format from " << candidates.size()
		<< " candidates with tiling: " << tiling
		<< " and features: " << features << "...";

	for (VkFormat format: candidates) {
		this->getLogger().info() << "Checking candidate format: " << format;

		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR
			&& (props.linearTilingFeatures & features) == features) {
			this->getLogger().info() << "Selected supported format: " << format
									 << " with linear tiling.";
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL
				   && (props.optimalTilingFeatures & features) == features) {
			this->getLogger().info() << "Selected supported format: " << format
									 << " with optimal tiling.";
			return format;
		}
	}

	this->getLogger().error()
		<< "Failed to find supported format from candidates with tiling: "
		<< tiling << " and features: " << features << ".";
	this->getLogger().warning()
		<< "Returning VK_FORMAT_UNDEFINED to indicate failure to find a "
		   "supported format. Ensure that the candidates provided are "
		   "compatible with the physical device and meet the required tiling "
		   "and feature criteria.";
	return VK_FORMAT_UNDEFINED;
}

utility::math::Vector2F evan::ASwapchainContext::getViewportSize() const
{
	utility::math::Vector2F viewportSize { 0.0f, 0.0f };
	if (_swapchainImages.empty()) {
		this->getLogger().warning()
			<< "No swapchain images available. Returning zero viewport size.";
		return viewportSize;
	}
	VkExtent2D extent = _swapchainImages[0]->getExtent();
	viewportSize.x	  = static_cast<float>(extent.width);
	viewportSize.y	  = static_cast<float>(extent.height);
	getLogger().info()
		<< "Retrieved viewport size from swapchain image: Width = "
		<< viewportSize.x << ", Height = " << viewportSize.y;
	return viewportSize;
}