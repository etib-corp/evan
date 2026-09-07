/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrSwapchainImage
*/

#include "evan/openxr/XrSwapchainImage.hpp"

evan::XrSwapchainImage::XrSwapchainImage(
	const CreateXrSwapchainImageProperties &properties)
{
	this->getLogger().info() << "Initializing XrSwapchainImage";

	uint32_t swapchainImageCount = 0;
	xrEnumerateSwapchainImages(properties.swapchain, 0, &swapchainImageCount,
							   nullptr);

	_swapchain = properties.swapchain;
	_device	   = properties.deviceContext.getDeviceBackend()->_device;
	_height	   = properties.createInfo.height;
	_width	   = properties.createInfo.width;

	_format = static_cast<VkFormat>(properties.createInfo.format);
	_extent = { properties.createInfo.width, properties.createInfo.height };

	_swapchainImages.resize(swapchainImageCount,
							{ XR_TYPE_SWAPCHAIN_IMAGE_VULKAN2_KHR });
	_imageViews.resize(swapchainImageCount);
	_framebuffers.resize(swapchainImageCount);

	_swapchainImagesBase =
		reinterpret_cast<XrSwapchainImageBaseHeader *>(&_swapchainImages[0]);

	xrEnumerateSwapchainImages(properties.swapchain, swapchainImageCount,
							   &swapchainImageCount, _swapchainImagesBase);

	for (auto &image: _swapchainImages) {
		image.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
	}
	for (size_t i = 0; i < _swapchainImages.size(); ++i) {
		_imageViews[i] =
			properties.deviceContext.getDeviceBackend()->createImageView(
				_swapchainImages[i].image, _format, VK_IMAGE_ASPECT_COLOR_BIT,
				1);
	}

	this->createColorResources(*properties.deviceContext.getDeviceBackend(),
							   properties.deviceContext.getMsaaSamples());
	this->createDepthResources(properties.deviceContext);
	this->createFramebuffers(
		properties.deviceContext.getDeviceBackend()->_device,
		properties.renderPass);
}

////////////////////
// Public Methods //
////////////////////

evan::XrSwapchainImage::~XrSwapchainImage()
{
	this->getLogger().info() << "Destroying XrSwapchainImage...";
	this->cleanup();
}

void evan::XrSwapchainImage::destroy(VkDevice device)
{
	(void)device;
	this->cleanup();
}

///////////////////////
// Private Methods   //
///////////////////////

void evan::XrSwapchainImage::cleanup()
{
	this->getLogger().info()
		<< "Destroying XrSwapchainImage and releasing resources";

	if (_device != VK_NULL_HANDLE) {
		for (size_t i = 0; i < _swapchainImages.size(); ++i) {
			this->getLogger().info()
				<< "Destroying image view and framebuffer for swapchain "
				   "image index: "
				<< i;
			if (i < _imageViews.size()
				&& _imageViews[i] != VK_NULL_HANDLE) {
				vkDestroyImageView(_device, _imageViews[i], nullptr);
			}
			if (i < _framebuffers.size()
				&& _framebuffers[i] != VK_NULL_HANDLE) {
				vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
			}
		}
	}

	_imageViews.clear();
	_framebuffers.clear();

	if (_device != VK_NULL_HANDLE) {
		this->getLogger().info()
			<< "Destroying color and depth resources for swapchain image";
		if (_colorView != VK_NULL_HANDLE) {
			vkDestroyImageView(_device, _colorView, nullptr);
			_colorView = VK_NULL_HANDLE;
		}
		if (_colorImage != VK_NULL_HANDLE) {
			vkDestroyImage(_device, _colorImage, nullptr);
			_colorImage = VK_NULL_HANDLE;
		}
		if (_colorMemory != VK_NULL_HANDLE) {
			vkFreeMemory(_device, _colorMemory, nullptr);
			_colorMemory = VK_NULL_HANDLE;
		}
		if (_depthView != VK_NULL_HANDLE) {
			vkDestroyImageView(_device, _depthView, nullptr);
			_depthView = VK_NULL_HANDLE;
		}
		if (_depthImage != VK_NULL_HANDLE) {
			vkDestroyImage(_device, _depthImage, nullptr);
			_depthImage = VK_NULL_HANDLE;
		}
		if (_depthMemory != VK_NULL_HANDLE) {
			vkFreeMemory(_device, _depthMemory, nullptr);
			_depthMemory = VK_NULL_HANDLE;
		}
	}

	if (_swapchain != XR_NULL_HANDLE) {
		xrDestroySwapchain(_swapchain);
		_swapchain = XR_NULL_HANDLE;
	}
}

void evan::XrSwapchainImage::fillPresentInfo(
	VkPresentInfoKHR &presentInfo) const
{
	presentInfo.swapchainCount = 0;
	presentInfo.pSwapchains	   = nullptr;
}