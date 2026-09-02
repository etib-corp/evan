/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopSwapchainContext
*/

#include "evan/glfw/DesktopSwapchainContext.hpp"

#include "evan/DeviceContext.hpp"

std::unordered_map<GLFWwindow *, evan::DesktopSwapchainContext *>
	evan::DesktopSwapchainContext::_contexts {};

evan::DesktopSwapchainContext::DesktopSwapchainContext(
	const DeviceContext &deviceContext, GLFWwindow *window)
	: _referenceWindow(window)
{
	this->getLogger().info() << "Initializing DesktopSwapchainContext...";

	this->createRenderPass(deviceContext.getDeviceBackend(),
						   deviceContext.getMsaaSamples());

	this->getLogger().info()
		<< "Creating swapchain images for DesktopSwapchainContext...";
	_swapchainImages.push_back(std::make_shared<DesktopSwapchainImage>(
		deviceContext, window, _renderPass));

	_viewSet.resize(1);

	this->getLogger().info()
		<< "Registering GLFW framebuffer size callback for proactive swapchain "
		   "recreation...";
	_contexts[window] = this;
	glfwSetFramebufferSizeCallback(
		window, &DesktopSwapchainContext::framebufferSizeCallback);
}

evan::DesktopSwapchainContext::~DesktopSwapchainContext()
{
	this->getLogger().info() << "Destroying DesktopSwapchainContext...";
	if (_referenceWindow) {
		glfwSetFramebufferSizeCallback(_referenceWindow, nullptr);
		_contexts.erase(_referenceWindow);
	}
}

////////////////////
// Public Methods //
////////////////////

void evan::DesktopSwapchainContext::destroy(VkDevice device)
{
	this->getLogger().info() << "Destroying swapchain and associated resources "
								"for DesktopSwapchainContext...";

	this->getLogger().info()
		<< "Destroying render pass for DesktopSwapchainContext...";
	vkDestroyRenderPass(device, _renderPass, nullptr);

	this->getLogger().info()
		<< "Destroying swapchain images for DesktopSwapchainContext...";
	for (const auto &swapchainImage: _swapchainImages) {
		this->getLogger().info() << "Destroying swapchain image and releasing "
									"associated resources...";
		swapchainImage->destroy(device);
	}
}

void evan::DesktopSwapchainContext::recreateSwapchain(
	const DeviceContext &deviceContext, VkRenderPass renderpass)
{
	this->getLogger().info() << "Recreating swapchain and associated resources "
								"for DesktopSwapchainContext...";

	auto device = deviceContext.getDeviceBackend()->_device;

	this->getLogger().info()
		<< "Waiting for the device to be idle before recreating the "
		   "swapchain...";
	vkDeviceWaitIdle(device);

	VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;
	if (!_swapchainImages.empty()) {
		oldSwapchain =
			dynamic_cast<DesktopSwapchainImage *>(_swapchainImages[0].get())
				->_swapchain;
	}

	this->getLogger().info()
		<< "Creating new swapchain image with the previous swapchain as "
		   "oldSwapchain...";
	auto newSwapchainImage = std::make_shared<DesktopSwapchainImage>(
		deviceContext, _referenceWindow, renderpass, oldSwapchain);

	this->getLogger().info() << "Destroying existing swapchain images for "
								"DesktopSwapchainContext...";
	for (const auto &swapchainImage: _swapchainImages) {
		this->getLogger().info() << "Destroying swapchain image and releasing "
									"associated resources...";
		swapchainImage->destroy(device);
	}
	_swapchainImages.clear();
	_swapchainImages.push_back(newSwapchainImage);

	this->getLogger().info()
		<< "Updating view for the new swapchain extent while preserving the "
		   "camera transform...";
	auto view = this->getViewSet()[0].view;
	ASwapchainContext::updateViewForExtent(
		view, newSwapchainImage->getExtent());
	this->getViewSet().setView(0, view);

	_framebufferResized = false;
}

VkResult evan::DesktopSwapchainContext::aquireImage(
	uint32_t index, VkDevice device, VkSemaphore imageAvailableSemaphore,
	VkFence inFlightFence, uint32_t &imageIndex)
{
	this->getLogger().info() << "Acquiring next available image from swapchain "
								"for DesktopSwapchainContext...";
	VkSwapchainKHR swapchain =
		dynamic_cast<DesktopSwapchainImage *>(_swapchainImages[index].get())
			->_swapchain;

	this->getLogger().info()
		<< "Calling vkAcquireNextImageKHR to acquire image from swapchain...";
	return vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
								 imageAvailableSemaphore, inFlightFence,
								 &imageIndex);
}

evan::ViewSet &evan::DesktopSwapchainContext::getViewSet()
{
	return _viewSet;
}

const evan::ViewSet &evan::DesktopSwapchainContext::getViewSet() const
{
	return _viewSet;
}

bool evan::DesktopSwapchainContext::usesImageAvailableSemaphore() const
{
	return true;
}

bool evan::DesktopSwapchainContext::needsSwapchainRecreation() const
{
	return _framebufferResized;
}

void evan::DesktopSwapchainContext::framebufferSizeCallback(
	GLFWwindow *window, int width, int height)
{
	(void)width;
	(void)height;
	auto it = _contexts.find(window);
	if (it != _contexts.end()) {
		it->second->_framebufferResized = true;
	}
}
