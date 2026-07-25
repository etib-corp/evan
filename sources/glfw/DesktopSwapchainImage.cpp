/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopSwapchainImage
*/

#include "evan/glfw/DesktopSwapchainImage.hpp"

#include <algorithm>
#include <limits>

#include "evan/DeviceContext.hpp"

evan::DesktopSwapchainImage::DesktopSwapchainImage(
	const DeviceContext &deviceContext, GLFWwindow *window,
	VkRenderPass renderpass)
{
	this->getLogger().info() << "Initializing DesktopSwapchainImage...";

	auto backend =
		(evan::DesktopBackend *)(deviceContext.getDeviceBackend().get());

	this->getLogger().info()
		<< "Querying swap chain support details for DesktopSwapchainImage...";
	evan::SwapChainSupportDetails swapChainSupport =
		backend->querySwapChainSupport();

	VkSurfaceFormatKHR surfaceFormat =
		this->chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode =
		this->chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent =
		this->chooseSwapExtent(swapChainSupport.capabilities, window);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0
		&& imageCount > swapChainSupport.capabilities.maxImageCount) {
		this->getLogger().warning()
			<< "Desired image count exceeds maximum supported by the swap "
			   "chain capabilities. Clamping to maxImageCount.";
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
		this->getLogger().info()
			<< "Graphics and present queue families are different. Using "
			   "concurrent sharing mode for swapchain images.";
		createInfo.imageSharingMode		 = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices	 = queueFamilyIndices;
	} else {
		this->getLogger().info()
			<< "Graphics and present queue families are the same. Using "
			   "exclusive sharing mode for swapchain images.";
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform	  = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	createInfo.presentMode	  = presentMode;
	createInfo.clipped		  = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	this->getLogger().info()
		<< "Creating swapchain with the following parameters:";
	this->getLogger().info()
		<< "Image Count = " << imageCount
		<< ", Image Format = " << surfaceFormat.format
		<< ", Color Space = " << surfaceFormat.colorSpace
		<< ", Image Extent = (" + std::to_string(extent.width) + ", "
			+ std::to_string(extent.height) + ")"
		<< ", Image Usage = " << createInfo.imageUsage << ", Sharing Mode = "
		<< (createInfo.imageSharingMode == VK_SHARING_MODE_CONCURRENT
				? "Concurrent"
				: "Exclusive")
		<< ", Pre Transform = " << createInfo.preTransform
		<< ", Composite Alpha = " << createInfo.compositeAlpha
		<< ", Present Mode = " << createInfo.presentMode
		<< ", Clipped = " << (createInfo.clipped ? "True" : "False")
		<< ", Old Swapchain = "
		<< (createInfo.oldSwapchain != VK_NULL_HANDLE ? "Valid Handle"
													  : "None");
	if (createInfo.oldSwapchain != VK_NULL_HANDLE) {
		this->getLogger().info()
			<< "Old swapchain handle provided. This indicates that the "
			   "swapchain is being recreated, likely due to a window resize or "
			   "similar event. The old swapchain will be automatically cleaned "
			   "up by the Vulkan implementation once the new swapchain is "
			   "created successfully.";
	}

	if (vkCreateSwapchainKHR(backend->_device, &createInfo, nullptr,
							 &_swapchain)
		!= VK_SUCCESS) {
		this->getLogger().error()
			<< "Failed to create swap chain for DesktopSwapchainImage!";
		return;
	}

	this->getLogger().info()
		<< "Swapchain created successfully for DesktopSwapchainImage.";

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
	this->getLogger().info() << "Destroying DesktopSwapchainImage...";
}

////////////////////
// Public Methods //
////////////////////

void evan::DesktopSwapchainImage::destroy(VkDevice device)
{
	this->getLogger().info()
		<< "Destroying swapchain image and releasing associated resources for "
		   "DesktopSwapchainImage...";

	this->getLogger().info()
		<< "Destroying framebuffers for DesktopSwapchainImage...";
	for (auto framebuffer: _framebuffers) {
		this->getLogger().info()
			<< "Destroying framebuffer and releasing associated resources...";
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	this->getLogger().info()
		<< "Destroying image views for DesktopSwapchainImage...";
	for (auto imageView: _imageViews) {
		this->getLogger().info()
			<< "Destroying image view and releasing associated resources...";
		vkDestroyImageView(device, imageView, nullptr);
	}

	this->getLogger().info()
		<< "Destroying swapchain for DesktopSwapchainImage...";
	vkDestroySwapchainKHR(device, _swapchain, nullptr);

	this->getLogger().info()
		<< "Destroying color resources for DesktopSwapchainImage...";
	vkDestroyImageView(device, _colorView, nullptr);
	vkDestroyImage(device, _colorImage, nullptr);
	vkFreeMemory(device, _colorMemory, nullptr);

	this->getLogger().info()
		<< "Destroying depth resources for DesktopSwapchainImage...";
	vkDestroyImageView(device, _depthView, nullptr);
	vkDestroyImage(device, _depthImage, nullptr);
	vkFreeMemory(device, _depthMemory, nullptr);
}

void evan::DesktopSwapchainImage::fillPresentInfo(
	VkPresentInfoKHR &presentInfo) const
{
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains	   = &_swapchain;
}

/////////////////////
// Private Methods //
/////////////////////

VkSurfaceFormatKHR evan::DesktopSwapchainImage::chooseSwapSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	this->getLogger().info()
		<< "Choosing swap surface format for DesktopSwapchainImage...";
	constexpr VkFormat kPreferredSwapchainFormats[] = {
		VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM
	};

	for (const auto &availableFormat: availableFormats) {
		for (VkFormat preferredFormat: kPreferredSwapchainFormats) {
			if (availableFormat.format == preferredFormat
				&& availableFormat.colorSpace
					== VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				this->getLogger().info()
					<< "Selected preferred swap surface format: Format = "
					<< availableFormat.format
					<< ", Color Space = " << availableFormat.colorSpace;
				return availableFormat;
			}
		}
	}
	this->getLogger().info()
		<< "No preferred swap surface format found, using default format.";
	return availableFormats[0];
}

VkPresentModeKHR evan::DesktopSwapchainImage::chooseSwapPresentMode(
	const std::vector<VkPresentModeKHR> &availablePresentModes)
{
	this->getLogger().info()
		<< "Choosing swap present mode for DesktopSwapchainImage...";
	for (const auto &availablePresentMode: availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			this->getLogger().info() << "Selected preferred swap present mode: "
										"VK_PRESENT_MODE_MAILBOX_KHR";
			return availablePresentMode;
		}
	}
	this->getLogger().info()
		<< "No preferred swap present mode found, using default mode.";
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D evan::DesktopSwapchainImage::chooseSwapExtent(
	const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window)
{
	this->getLogger().info()
		<< "Choosing swap extent for DesktopSwapchainImage...";
	int width  = 0;
	int height = 0;

	if (capabilities.currentExtent.width
		!= (std::numeric_limits<uint32_t>::max)()) {
		this->getLogger().info() << "Current extent is defined by the surface "
									"capabilities. Using current extent: ("
				+ std::to_string(capabilities.currentExtent.width) + ", "
				+ std::to_string(capabilities.currentExtent.height) + ")";
		return capabilities.currentExtent;
	}

	this->getLogger().info() << "Current extent is undefined. Querying "
								"framebuffer size from GLFW window...";
	glfwGetFramebufferSize(window, &width, &height);

	VkExtent2D actualExtent = { static_cast<uint32_t>(width),
								static_cast<uint32_t>(height) };

	this->getLogger().info() << "Queried framebuffer size: ("
			+ std::to_string(actualExtent.width) + ", "
			+ std::to_string(actualExtent.height)
			+ "). Clamping to allowed extent range defined by surface "
			  "capabilities...";

	actualExtent.width =
		std::clamp(actualExtent.width, capabilities.minImageExtent.width,
				   capabilities.maxImageExtent.width);
	actualExtent.height =
		std::clamp(actualExtent.height, capabilities.minImageExtent.height,
				   capabilities.maxImageExtent.height);

	this->getLogger().info() << "Final swap extent after clamping: ("
			+ std::to_string(actualExtent.width) + ", "
			+ std::to_string(actualExtent.height) + ")";

	return actualExtent;
}
