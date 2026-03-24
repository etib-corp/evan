/*
** ETIB PROJECT, 2026
** evan
** File description:
** ASwapchainImage
*/

#include "ASwapchainImage.hpp"

#include "DeviceContext.hpp"

evan::ASwapchainImage::~ASwapchainImage()
{
}

VkFormat evan::ASwapchainImage::findDepthFormat(VkPhysicalDevice physicalDevice)
{
	return evan::ASwapchainImage::findSupportedFormat(physicalDevice,
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

/////////////
// Getters //
/////////////

VkFramebuffer evan::ASwapchainImage::getFramebuffer(uint32_t index) const
{
	return _framebuffers[index];
}

VkExtent2D evan::ASwapchainImage::getExtent() const
{
	return _extent;
}

uint32_t evan::ASwapchainImage::getFramebufferCount() const
{
	return static_cast<uint32_t>(_framebuffers.size());
}

///////////////////////
// Protected methods //
///////////////////////

void evan::ASwapchainImage::createImageViews(
	const ADeviceBackend &deviceBackend)
{
	_imageViews.resize(_images.size());

	for (uint32_t i = 0; i < _images.size(); i++) {
		_imageViews[i] = deviceBackend.createImageView(
			_images[i], _format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

void evan::ASwapchainImage::createColorResources(
	const ADeviceBackend &deviceBackend, VkSampleCountFlagBits msaaSamples)
{
	VkFormat colorFormat								  = _format;
	ADeviceBackend::CreateImageProperties imageProperties = {
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
	_colorView = deviceBackend.createImageView(_colorImage, colorFormat,
											   VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void evan::ASwapchainImage::createDepthResources(
	const DeviceContext &deviceContext)
{
	VkPhysicalDevice physicalDevice =
		deviceContext.getDeviceBackend()->_physicalDevice;

	VkSampleCountFlagBits msaaSamples = deviceContext.getMsaaSamples();
	VkCommandPool commandPool		  = deviceContext.getCommandPool();
	VkQueue graphicsQueue			  = deviceContext.getGraphicsQueue();

	VkFormat depthFormat = this->findDepthFormat(physicalDevice);
	ADeviceBackend::CreateImageProperties depthImageProperties = {
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
	_depthView = deviceContext.getDeviceBackend()->createImageView(
		_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	ADeviceBackend::TransitionImageLayoutProperties transitionProperties = {
		._commandPool	= commandPool,
		._graphicsQueue = graphicsQueue,
		._image			= _depthImage,
		._format		= depthFormat,
		._oldLayout		= VK_IMAGE_LAYOUT_UNDEFINED,
		._newLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		._mipLevels		= 1	   // No mipmaps for depth attachment
	};

	deviceContext.getDeviceBackend()->transitionImageLayout(
		transitionProperties);
}

void evan::ASwapchainImage::createFramebuffers(VkDevice logicalDevice,
											   VkRenderPass renderPass)
{
	_framebuffers.resize(_imageViews.size());

	for (size_t i = 0; i < _imageViews.size(); i++) {
		std::array<VkImageView, 3> attachments = { _colorView, _depthView,
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

void evan::ASwapchainImage::createImages(VkDevice logicalDevice,
										 VkSwapchainKHR swapchain)
{
	uint32_t imageCount;

	vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
	_images.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount,
							_images.data());
}
