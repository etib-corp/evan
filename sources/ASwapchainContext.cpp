/*
** EPITECH PROJECT, 2026
** evan
** File description:
** SwapchainContext
*/

#include "ASwapchainContext.hpp"

void evan::ASwapchainContext::createRenderPass(
	const std::shared_ptr<ADeviceBackend> &deviceBackend,
	VkSampleCountFlagBits msaaSamples)
{
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

	VkAttachmentReference colorAttachmentRef {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout	  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve {};
	colorAttachmentResolve.format		  = swapchainFormat;
	colorAttachmentResolve.samples		  = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp		  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp		  = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout	  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentResolveRef {};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass {};
	subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount	= 1;
	subpass.pColorAttachments		= &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments		= &colorAttachmentResolveRef;

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

	if (vkCreateRenderPass(deviceBackend->_device, &renderPassInfo, nullptr,
						   &_renderPass)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

VkRenderPass evan::ASwapchainContext::getRenderPass() const
{
	return _renderPass;
}

VkFormat evan::ASwapchainContext::selectSwapchainFormat(
	const std::vector<int64_t> &swapchainFormats)
{
	constexpr VkFormat kPreferredSwapchainFormats[] = {
		VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM
	};

	auto swapchainFormatIt =
		std::find_first_of(swapchainFormats.begin(), swapchainFormats.end(),
						   std::begin(kPreferredSwapchainFormats),
						   std::end(kPreferredSwapchainFormats));

	if (swapchainFormatIt == swapchainFormats.end()) {
		std::cerr
			<< "SwapchainContext: No preferred swapchain format found, using "
			   "first available format"
			<< std::endl;
		return static_cast<VkFormat>(swapchainFormats[0]);
	}
	return static_cast<VkFormat>(*swapchainFormatIt);
}

VkFormat evan::ASwapchainContext::findSupportedFormat(
	VkPhysicalDevice physicalDevice, const std::vector<VkFormat> &candidates,
	VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format: candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR
			&& (props.linearTilingFeatures & features) == features) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL
				   && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}
