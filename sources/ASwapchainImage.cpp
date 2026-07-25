/*
** ETIB PROJECT, 2026
** evan
** File description:
** ASwapchainImage
*/

#include "evan/ASwapchainImage.hpp"

#include "evan/DeviceContext.hpp"

evan::ASwapchainImage::~ASwapchainImage()
{
}

VkFormat evan::ASwapchainImage::findDepthFormat(VkPhysicalDevice physicalDevice)
{
	return evan::ASwapchainImage::findSupportedFormat(
		physicalDevice,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
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

	return VK_FORMAT_UNDEFINED;
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
	this->getLogger().info()
		<< "Creating image views for swapchain images with format: " << _format
		<< " and aspect mask: " << VK_IMAGE_ASPECT_COLOR_BIT << "...";

	_imageViews.resize(_images.size());

	this->getLogger().info()
		<< "Number of image views to create: " << _imageViews.size();

	for (uint32_t i = 0; i < _images.size(); i++) {
		this->getLogger().info()
			<< "Creating image view for swapchain image " << i
			<< " with image handle: " << (uintptr_t)_images[i];
		_imageViews[i] = deviceBackend.createImageView(
			_images[i], _format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

void evan::ASwapchainImage::createColorResources(
	const ADeviceBackend &deviceBackend, VkSampleCountFlagBits msaaSamples)
{
	this->getLogger().info()
		<< "Creating color resources for swapchain images with format: "
		<< _format << " and MSAA samples: " << msaaSamples << "...";

	VkFormat colorFormat								  = _format;
	ADeviceBackend::CreateImageProperties imageProperties = {
		._width		 = _extent.width,
		._height	 = _extent.height,
		._mipLevels	 = 1,	 // No mipmaps for color attachment
		._numSamples = msaaSamples,
		._format	 = colorFormat,
		._tiling	 = VK_IMAGE_TILING_OPTIMAL,
		._usage		 = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		._properties  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._image		  = _colorImage,
		._imageMemory = _colorMemory
	};

	this->getLogger().info()
		<< "Creating color image using:\n color format: " << colorFormat
		<< "\n width: " << _extent.width << "\n height: " << _extent.height
		<< "\n mip levels: " << 1 << "\n num samples: " << msaaSamples
		<< "\n tiling: " << VK_IMAGE_TILING_OPTIMAL << "\n usage: "
		<< (VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		<< "\n properties: " << VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	deviceBackend.createImage(imageProperties);
	_colorView = deviceBackend.createImageView(_colorImage, colorFormat,
											   VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void evan::ASwapchainImage::createDepthResources(
	const DeviceContext &deviceContext)
{
	this->getLogger().info()
		<< "Creating depth resources for swapchain images...";

	VkPhysicalDevice physicalDevice =
		deviceContext.getDeviceBackend()->_physicalDevice;

	VkSampleCountFlagBits msaaSamples = deviceContext.getMsaaSamples();
	VkCommandPool commandPool		  = deviceContext.getCommandPool();
	VkQueue graphicsQueue			  = deviceContext.getGraphicsQueue();

	this->getLogger().info() << "MSAA samples: " << msaaSamples;
	this->getLogger().info() << "Finding supported depth format...";

	VkFormat depthFormat = this->findDepthFormat(physicalDevice);

	if (depthFormat == VK_FORMAT_UNDEFINED) {
		this->getLogger().error() << "Failed to find a supported depth format!";
		return;
	}

	this->getLogger().info() << "Found supported depth format: " << depthFormat;

	ADeviceBackend::CreateImageProperties depthImageProperties = {
		._width		  = _extent.width,
		._height	  = _extent.height,
		._mipLevels	  = 1,	  // No mipmaps for depth attachment
		._numSamples  = msaaSamples,
		._format	  = depthFormat,
		._tiling	  = VK_IMAGE_TILING_OPTIMAL,
		._usage		  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		._properties  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._image		  = _depthImage,
		._imageMemory = _depthMemory
	};

	this->getLogger().info()
		<< "Creating depth image using:\n depth format: " << depthFormat
		<< "\n width: " << _extent.width << "\n height: " << _extent.height
		<< "\n mip levels: " << 1 << "\n num samples: " << msaaSamples
		<< "\n tiling: " << VK_IMAGE_TILING_OPTIMAL
		<< "\n usage: " << VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
		<< "\n properties: " << VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	deviceContext.getDeviceBackend()->createImage(depthImageProperties);
	_depthView = deviceContext.getDeviceBackend()->createImageView(
		_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	this->getLogger().info() << "Transitioning depth image layout to "
								"DEPTH_STENCIL_ATTACHMENT_OPTIMAL...";

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

	this->getLogger().info() << "Depth resources creation terminated.";
}

void evan::ASwapchainImage::createFramebuffers(VkDevice logicalDevice,
											   VkRenderPass renderPass)
{
	this->getLogger().info() << "Creating framebuffers for swapchain images...";

	_framebuffers.resize(_imageViews.size());

	this->getLogger().info()
		<< "Number of framebuffers to create: " << _framebuffers.size();

	for (size_t i = 0; i < _imageViews.size(); i++) {
		this->getLogger().info()
			<< "Creating framebuffer " << i
			<< " with color view: " << (uintptr_t)_colorView
			<< ", depth view: " << (uintptr_t)_depthView
			<< ", image view: " << (uintptr_t)_imageViews[i];

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

		this->getLogger().info()
			<< "Framebuffer creation info:\n render pass: "
			<< (uintptr_t)renderPass
			<< ", attachment count: " << framebufferInfo.attachmentCount
			<< ", width: " << framebufferInfo.width
			<< ", height: " << framebufferInfo.height
			<< ", layers: " << framebufferInfo.layers;

		if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr,
								&_framebuffers[i])
			!= VK_SUCCESS) {
			this->getLogger().error()
				<< "Failed to create framebuffer " << i << "!";
			this->getLogger().warning() << "Skipping framebuffer " << i
										<< " and continuing with next one...";
			continue;
		}
	}
}

void evan::ASwapchainImage::createImages(VkDevice logicalDevice,
										 VkSwapchainKHR swapchain)
{
	this->getLogger().info() << "Retrieving swapchain images...";

	uint32_t imageCount;

	vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
	_images.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount,
							_images.data());

	this->getLogger().info()
		<< "Number of swapchain images retrieved: " << imageCount;
}
