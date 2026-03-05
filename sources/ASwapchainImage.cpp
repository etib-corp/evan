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


/**
 * @brief Creates a Vulkan image view for a given image.
 *
 * This function sets up and creates a Vulkan image view, which is used to
 * describe how an image resource should be accessed. It specifies the format,
 * view type, and subresource range for the image view.
 *
 * @param image The Vulkan image for which the image view is created.
 * @param format The format of the image view (e.g., VK_FORMAT_R8G8B8A8_SRGB).
 * @param aspectFlags Specifies which aspect(s) of the image are included in the
 * view (e.g., VK_IMAGE_ASPECT_COLOR_BIT for color images).
 * @param logicalDevice The Vulkan logical device used to create the image view.
 *
 * @return A VkImageView handle representing the created image view.
 *
 * @throws std::runtime_error If the image view creation fails.
 */

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
