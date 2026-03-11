/*
** ETIB PROJECT, 2026
** evan
** File description:
** ASwapchainImage
*/

#include "ASwapchainImage.hpp"

evan::ASwapchainImage::~ASwapchainImage()
{
}

void evan::ASwapchainImage::createImageViews(VkDevice logicalDevice)
{
	_imageViews.resize(_images.size());

	for (uint32_t i = 0; i < _images.size(); i++) {
		_imageViews[i] = this->createImageView(
			_images[i], _format, VK_IMAGE_ASPECT_COLOR_BIT, logicalDevice, 1);
	}
}
void evan::ASwapchainImage::createColorResources(const ADeviceBackend &deviceBackend, VkSampleCountFlagBits msaaSamples)
{
	VkFormat colorFormat				  = _format;
	ADeviceBackend::CreateImageProperties imageProperties = {
		._logicalDevice	 = deviceBackend._device,
		._physicalDevice = deviceBackend._physicalDevice,
		._width			 = _extent.width,
		._height		 = _extent.height,
		._mipLevels		 = 1,	 // No mipmaps for color attachment
		._numSamples	 = msaaSamples,
		._format		 = colorFormat,
		._tiling		 = VK_IMAGE_TILING_OPTIMAL,
		._usage			 = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		._properties  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._image		  = _colorImage,
		._imageMemory = _colorMemory
	};

	deviceBackend.createImage(imageProperties);
	_colorView = this->createImageView(
		_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, deviceBackend._device, 1);
}

void evan::ASwapchainImage::createDepthResources(
	const DeviceContext &deviceContext)
{
	VkPhysicalDevice physicalDevice =
		deviceContext.getDeviceBackend()->_physicalDevice;
	VkDevice logicalDevice = deviceContext.getDeviceBackend()->_device;

	VkSampleCountFlagBits msaaSamples = deviceContext.getMsaaSamples();
	VkCommandPool commandPool		  = deviceContext.getCommandPool();
	VkQueue graphicsQueue			  = deviceContext.getGraphicsQueue();

	VkFormat depthFormat = this->findDepthFormat(physicalDevice);
	ADeviceBackend::CreateImageProperties depthImageProperties = {
		._logicalDevice	 = logicalDevice,
		._physicalDevice = physicalDevice,
		._width			 = _extent.width,
		._height		 = _extent.height,
		._mipLevels		 = 1,	 // No mipmaps for depth attachment
		._numSamples	 = msaaSamples,
		._format		 = depthFormat,
		._tiling		 = VK_IMAGE_TILING_OPTIMAL,
		._usage			 = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		._properties	 = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._image			 = _depthImage,
		._imageMemory	 = _depthMemory
	};

	deviceContext.getDeviceBackend()->createImage(depthImageProperties);
	_depthView = this->createImageView(
		_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, logicalDevice, 1);

	TransitionImageLayoutProperties transitionProperties = {
		._logicalDevice = logicalDevice,
		._commandPool	= commandPool,
		._graphicsQueue = graphicsQueue,
		._image			= _depthImage,
		._format		= depthFormat,
		._oldLayout		= VK_IMAGE_LAYOUT_UNDEFINED,
		._newLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		._mipLevels		= 1	   // No mipmaps for depth attachment
	};

	this->transitionImageLayout(transitionProperties);
}

void evan::ASwapchainImage::createFramebuffers(VkDevice logicalDevice,
											   VkRenderPass renderPass)
{
	_framebuffers.resize(_imageViews.size());

	for (size_t i = 0; i < _imageViews.size(); i++) {
		std::array<VkImageView, 3> attachments = { _colorView,
												   _depthView,
												   _imageViews[i] };

		VkFramebufferCreateInfo framebufferInfo {};
		framebufferInfo.sType	   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount =
			static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width		 = _extent.width;
		framebufferInfo.height		 = _extent.height;
		framebufferInfo.layers		 = 1;

		if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr,
								&_framebuffers[i])
			!= VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

void evan::ASwapchainImage::createImages(VkDevice logicalDevice, VkSwapchainKHR swapchain)
{
	uint32_t imageCount;

	vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
	_images.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount,
							_images.data());
}

VkImageView evan::ASwapchainImage::createImageView(
	VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
	VkDevice logicalDevice, uint32_t mipLevels)
{
	VkImageViewCreateInfo viewInfo {};
	viewInfo.sType	  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image	  = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format	  = format;
	viewInfo.subresourceRange.aspectMask	 = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel	 = 0;
	viewInfo.subresourceRange.levelCount	 = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount	 = 1;
	viewInfo.subresourceRange.levelCount	 = mipLevels;

	VkImageView imageView;
	if (vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}
///////////////////////
// Private functions //
///////////////////////


VkFormat evan::ASwapchainImage::findDepthFormat(VkPhysicalDevice physicalDevice)
{
	return findSupportedFormat(physicalDevice,
							   { VK_FORMAT_D32_SFLOAT,
								 VK_FORMAT_D32_SFLOAT_S8_UINT,
								 VK_FORMAT_D24_UNORM_S8_UINT },
							   VK_IMAGE_TILING_OPTIMAL,
							   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat evan::ASwapchainImage::findSupportedFormat(
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

void evan::ASwapchainImage::transitionImageLayout(
	const TransitionImageLayoutProperties &properties)
{
	VkCommandBuffer commandBuffer = this->beginSingleTimeCommands(
		properties._logicalDevice, properties._commandPool);

	VkImageMemoryBarrier barrier {};
	barrier.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout			= properties._oldLayout;
	barrier.newLayout			= properties._newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image				= properties._image;
	barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel	= 0;
	barrier.subresourceRange.levelCount		= 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount		= 1;
	barrier.subresourceRange.levelCount		= properties._mipLevels;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (properties._oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
		&& properties._newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (properties._oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			   && properties._newLayout
				   == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (properties._oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
			   && properties._newLayout
				   == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	if (properties._newLayout
		== VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (this->hasStencilComponent(properties._format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
						 nullptr, 0, nullptr, 1, &barrier);

	this->endSingleTimeCommands(properties._logicalDevice,
								properties._commandPool,
								properties._graphicsQueue, commandBuffer);
}

VkCommandBuffer
	evan::ASwapchainImage::beginSingleTimeCommands(VkDevice logicalDevice,
												   VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocInfo {};
	allocInfo.sType		  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level		  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer)
		!= VK_SUCCESS) {
		std::cerr << "Failed to allocate command buffer" << std::endl;
		return VK_NULL_HANDLE;
	}

	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

bool evan::ASwapchainImage::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT
		|| format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void evan::ASwapchainImage::endSingleTimeCommands(VkDevice logicalDevice,
												  VkCommandPool commandPool,
												  VkQueue graphicsQueue,
												  VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo {};
	submitInfo.sType			  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers	  = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}
