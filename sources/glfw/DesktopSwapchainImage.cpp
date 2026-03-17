/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopSwapchainImage
*/

#include "glfw/DesktopSwapchainImage.hpp"

evan::DesktopSwapchainImage::DesktopSwapchainImage(
	const DeviceContext &deviceContext, GLFWwindow *window, VkRenderPass renderpass)
{
	auto backend =
		(evan::DesktopBackend *)(deviceContext.getDeviceBackend().get());
	evan::SwapChainSupportDetails swapChainSupport =
		backend->querySwapChainSupport(backend->_physicalDevice,
									   backend->_surface);

	VkSurfaceFormatKHR surfaceFormat =
		this->chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode =
		this->chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent =
		this->chooseSwapExtent(swapChainSupport.capabilities, window);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0
		&& imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo {};
	createInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = backend->_surface;

	createInfo.minImageCount	= imageCount;
	createInfo.imageFormat		= surfaceFormat.format;
	createInfo.imageColorSpace	= surfaceFormat.colorSpace;
	createInfo.imageExtent		= extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	evan::QueueFamilyIndices indices = backend->findQueueFamilies();
	uint32_t queueFamilyIndices[]	 = { indices.graphicsFamily.value(),
										 indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode		 = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices	 = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform	  = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	createInfo.presentMode	  = presentMode;
	createInfo.clipped		  = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(backend->_device, &createInfo, nullptr,
							 &_swapchain)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	_extent = extent;
    _format = surfaceFormat.format;

    this->createImages(backend->_device, _swapchain);
    this->createImageViews(*backend);
    this->createColorResources(*backend, deviceContext.getMsaaSamples());
    this->createDepthResources(deviceContext);
    this->createFramebuffers(backend->_device, renderpass);
}

evan::DesktopSwapchainImage::~DesktopSwapchainImage()
{
}

void evan::DesktopSwapchainImage::destroy(VkDevice device)
{
	for (auto framebuffer: _framebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	for (auto imageView: _imageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	for (auto image: _images) {
		vkDestroyImage(device, image, nullptr);
	}

	vkDestroySwapchainKHR(device, _swapchain, nullptr);

	vkDestroyImageView(device, _colorView, nullptr);
	vkDestroyImage(device, _colorImage, nullptr);
	vkFreeMemory(device, _colorMemory, nullptr);

	vkDestroyImageView(device, _depthView, nullptr);
	vkDestroyImage(device, _depthImage, nullptr);
	vkFreeMemory(device, _depthMemory, nullptr);
}

/////////////////////
// Private Methods //
/////////////////////

VkSurfaceFormatKHR evan::DesktopSwapchainImage::chooseSwapSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	for (const auto &availableFormat: availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& availableFormat.colorSpace
				== VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR evan::DesktopSwapchainImage::chooseSwapPresentMode(
	const std::vector<VkPresentModeKHR> &availablePresentModes)
{
	for (const auto &availablePresentMode: availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D evan::DesktopSwapchainImage::chooseSwapExtent(
	const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window)
{
	int width  = 0;
	int height = 0;

	if (capabilities.currentExtent.width
		!= std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	glfwGetFramebufferSize(window, &width, &height);

	VkExtent2D actualExtent = { static_cast<uint32_t>(width),
								static_cast<uint32_t>(height) };

	actualExtent.width =
		std::clamp(actualExtent.width, capabilities.minImageExtent.width,
				   capabilities.maxImageExtent.width);
	actualExtent.height =
		std::clamp(actualExtent.height, capabilities.minImageExtent.height,
				   capabilities.maxImageExtent.height);

	return actualExtent;
}
