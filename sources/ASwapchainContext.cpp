/*
** ETIB PROJECT, 2026
** evan
** File description:
** SwapchainContext
*/

#include "evan/ASwapchainContext.hpp"

#include <cstdint>
#include <numbers>

void evan::ASwapchainContext::createRenderPass(
	const std::shared_ptr<ADeviceBackend> &deviceBackend,
	VkSampleCountFlagBits msaaSamples, ColorAttachmentMode mode)
{
	this->getLogger().info() << "Creating render pass for swapchain context...";

	// A resolve attachment is only valid when the color attachment is
	// multisampled. Desktop contexts pick DirectToSwapchain in that case, so
	// this only guards against a caller building an invalid render pass.
	if (mode == ColorAttachmentMode::ResolveToSwapchain
		&& msaaSamples == VK_SAMPLE_COUNT_1_BIT) {
		this->getLogger().warning()
			<< "A resolve attachment was requested with a single sample. "
			   "Rendering directly into the swapchain image instead.";
		mode = ColorAttachmentMode::DirectToSwapchain;
	}

	auto swapchainFormatCount = deviceBackend->countSwapchainFormats();
	auto swapchainFormats =
		deviceBackend->enumerateSwapchainFormats(swapchainFormatCount);
	auto swapchainFormat = selectSwapchainFormat(
		deviceBackend->getPhysicalDevice(), swapchainFormats);

	VkAttachmentDescription colorAttachment {};
	colorAttachment.format		   = swapchainFormat;
	colorAttachment.loadOp		   = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp		   = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.samples		   = msaaSamples;
	// When the swapchain image is the color attachment, the render pass must
	// hand it over ready to be presented. When a resolve target is used
	// instead, the color attachment is a transient multisampled image and the
	// resolve target ends in VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
	colorAttachment.finalLayout = mode == ColorAttachmentMode::DirectToSwapchain
		? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	this->getLogger().info()
		<< "Selected swapchain format: " << swapchainFormat;

	VkAttachmentDescription depthAttachment {};
	depthAttachment.format =
		ASwapchainImage::findDepthFormat(deviceBackend->getPhysicalDevice());
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
	VkAttachmentReference colorAttachmentResolveRef {};
	colorAttachmentResolveRef.attachment = 2;

	std::vector<VkAttachmentDescription> attachments = { colorAttachment,
														 depthAttachment };

	VkSubpassDescription subpass {};
	subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount	= 1;
	subpass.pColorAttachments		= &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	if (mode == ColorAttachmentMode::ResolveToSwapchain) {
		colorAttachmentResolve.format		 = swapchainFormat;
		colorAttachmentResolve.samples		 = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp		 = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp		 = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp =
			VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout	 = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		colorAttachmentResolveRef.layout =
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments.push_back(colorAttachmentResolve);
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		this->getLogger().info()
			<< "Configured color attachment resolve with format: "
			<< colorAttachmentResolve.format
			<< ", samples: " << colorAttachmentResolve.samples
			<< ", final layout: " << colorAttachmentResolve.finalLayout;
	} else {
		subpass.pResolveAttachments = nullptr;
		this->getLogger().info()
			<< (mode == ColorAttachmentMode::DirectToSwapchain
					? "Rendering directly into the swapchain image without a "
					  "resolve attachment."
					: "Rendering into the runtime swapchain image without a "
					  "resolve attachment.");
	}

	this->getLogger().info()
		<< "Configured subpass with pipeline bind point: "
		<< subpass.pipelineBindPoint
		<< ", color attachment count: " << subpass.colorAttachmentCount
		<< ", depth-stencil attachment reference: "
		<< depthAttachmentRef.attachment << ", and color attachment reference: "
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

	if (vkCreateRenderPass(deviceBackend->getDevice(), &renderPassInfo, nullptr,
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

std::size_t evan::ASwapchainContext::getSwapchainCount() const
{
	return _swapchainImages.size();
}

bool evan::ASwapchainContext::usesImageAvailableSemaphore() const
{
	return false;
}

bool evan::ASwapchainContext::needsSwapchainRecreation() const
{
	return false;
}

utility::graphic::ViewF
	evan::ASwapchainContext::getView(std::size_t index) const
{
	return getViewSet().getView(index);
}

void evan::ASwapchainContext::setView(std::size_t index,
									  const utility::graphic::ViewF &view)
{
	getViewSet().setView(index, view);
}

const utility::graphic::PoseF &
	evan::ASwapchainContext::getViewOffset(void) const
{
	return _viewOffset;
}

std::size_t evan::ASwapchainContext::getViewCount(void) const
{
	return getViewSet().size();
}

glm::mat4 evan::ASwapchainContext::getProjection(std::size_t index) const
{
	return getViewSet().getProjection(index);
}

VkFormat evan::ASwapchainContext::selectSwapchainFormat(
	VkPhysicalDevice physicalDevice,
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

	if (swapchainFormats.empty()) {
		this->getLogger().warning()
			<< "No swapchain formats reported by the runtime, using fallback "
			   "format.";
		return fallbackSwapchainFormat(physicalDevice);
	}

	for (int64_t candidate: swapchainFormats) {
		for (VkFormat preferred: kPreferredSwapchainFormats) {
			if (candidate == static_cast<int64_t>(preferred)) {
				this->getLogger().info()
					<< "Selected preferred swapchain format: " << preferred;
				return preferred;
			}
		}
	}

	for (int64_t candidate: swapchainFormats) {
		if (!isPlausibleVkFormat(candidate)) {
			this->getLogger().warning()
				<< "Ignoring invalid swapchain format value: " << candidate;
			continue;
		}

		VkFormat format = static_cast<VkFormat>(candidate);
		if (physicalDevice == VK_NULL_HANDLE
			|| supportsColorAttachment(physicalDevice, format)) {
			this->getLogger().info() << "Selected swapchain format: " << format;
			return format;
		}

		this->getLogger().warning()
			<< "Ignoring unsupported swapchain format: " << format;
	}

	this->getLogger().error()
		<< "No valid or supported swapchain format found, using fallback "
		   "format.";
	return fallbackSwapchainFormat(physicalDevice);
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

void evan::ASwapchainContext::updateViewForExtent(utility::graphic::ViewF &view,
												  VkExtent2D extent)
{
	const auto width  = static_cast<float>(extent.width);
	const auto height = static_cast<float>(extent.height);

	view.setViewportSize(utility::math::Vector2F { width, height });

	auto verticalFov = view.getVerticalFovRadians();
	if (verticalFov <= 0.0f) {
		verticalFov = std::numbers::pi_v<float> * 0.5f;
	}

	const float aspectRatio = height > 0.0f ? width / height : 1.0f;
	view.setPerspective(verticalFov, aspectRatio);
}

bool evan::ASwapchainContext::isPlausibleVkFormat(int64_t value)
{
	return value > static_cast<int64_t>(VK_FORMAT_UNDEFINED)
		&& value < static_cast<int64_t>(VK_FORMAT_MAX_ENUM);
}

bool evan::ASwapchainContext::supportsColorAttachment(
	VkPhysicalDevice physicalDevice, VkFormat format)
{
	VkFormatProperties properties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
	return (properties.optimalTilingFeatures
			& VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
		== VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
}

VkFormat evan::ASwapchainContext::fallbackSwapchainFormat(
	VkPhysicalDevice physicalDevice)
{
	constexpr VkFormat kFallbacks[] = { VK_FORMAT_B8G8R8A8_UNORM,
										VK_FORMAT_R8G8B8A8_UNORM };

	for (VkFormat format: kFallbacks) {
		if (physicalDevice == VK_NULL_HANDLE
			|| supportsColorAttachment(physicalDevice, format)) {
			return format;
		}
	}

	return VK_FORMAT_B8G8R8A8_UNORM;
}