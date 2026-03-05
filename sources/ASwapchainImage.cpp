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
			_images[i], _format,
			VK_IMAGE_ASPECT_COLOR_BIT, logicalDevice, 1);
	}
}

void evan::ASwapchainImage::createColorResources(
	VkDevice logicalDevice, VkPhysicalDevice physicalDevice,
	VkSampleCountFlagBits msaaSamples)
{
	(void)msaaSamples;
	VkFormat colorFormat						 = _format;
	CreateImageProperties imageProperties = {
		._logicalDevice	 = logicalDevice,
		._physicalDevice = physicalDevice,
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

	this->createImage(imageProperties);
	_colorView = this->createImageView(_colorImage, colorFormat,
											 VK_IMAGE_ASPECT_COLOR_BIT,
											 logicalDevice, 1);
}

VkImageView evan::ASwapchainImage::createImageView(VkImage image, VkFormat format,
										 VkImageAspectFlags aspectFlags,
										 VkDevice logicalDevice,
										 uint32_t mipLevels)
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

void evan::ASwapchainImage::createImage(const evan::ASwapchainImage::CreateImageProperties &properties)
{
	VkImageCreateInfo imageInfo {};
	imageInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType		= VK_IMAGE_TYPE_2D;
	imageInfo.extent.width	= properties._width;
	imageInfo.extent.height = properties._height;
	imageInfo.extent.depth	= 1;
	imageInfo.mipLevels		= 1;
	imageInfo.arrayLayers	= 1;
	imageInfo.format		= properties._format;
	imageInfo.tiling		= properties._tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage			= properties._usage;
	imageInfo.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.mipLevels		= properties._mipLevels;
	imageInfo.samples		= properties._numSamples;

	if (vkCreateImage(properties._logicalDevice, &imageInfo, nullptr,
					  &properties._image)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(properties._logicalDevice, properties._image,
								 &memRequirements);

	VkMemoryAllocateInfo allocInfo {};
	allocInfo.sType			  = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize  = memRequirements.size;
	allocInfo.memoryTypeIndex = this->findMemoryType(
		properties._physicalDevice, memRequirements.memoryTypeBits,
		properties._properties);

	if (vkAllocateMemory(properties._logicalDevice, &allocInfo, nullptr,
						 &properties._imageMemory)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory!");
	}

	vkBindImageMemory(properties._logicalDevice, properties._image,
					  properties._imageMemory, 0);
}

uint32_t evan::ASwapchainImage::findMemoryType(VkPhysicalDevice physicalDevice,
									 uint32_t typeFilter,
									 VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i))
			&& (memProperties.memoryTypes[i].propertyFlags & properties)
				== properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

