/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopSwapchainContext
*/

#include "evan/glfw/DesktopSwapchainContext.hpp"

#include "evan/DeviceContext.hpp"

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
}

evan::DesktopSwapchainContext::~DesktopSwapchainContext()
{
	this->getLogger().info() << "Destroying DesktopSwapchainContext...";
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

	this->getLogger().info() << "Destroying existing swapchain images for "
								"DesktopSwapchainContext...";
	for (const auto &swapchainImage: _swapchainImages) {
		this->getLogger().info() << "Destroying swapchain image and releasing "
									"associated resources...";
		swapchainImage->destroy(deviceContext.getDeviceBackend()->_device);
	}

	this->getLogger().info()
		<< "Creating new swapchain images for DesktopSwapchainContext...";
	_swapchainImages.clear();

	auto newSwapchainImage = std::make_shared<DesktopSwapchainImage>(
		deviceContext, _referenceWindow, renderpass);

	_swapchainImages.push_back(newSwapchainImage);

	auto viewportSize = this->getViewportSize();
	this->getLogger().error()
		<< "Setting up view for Desktop platform with viewport size: "
		<< viewportSize;

	auto view		 = this->getView(0);
	view.setViewportSize(
		utility::math::Vector2F{viewportSize.x / 2, viewportSize.y / 2});
	view.setPerspective(M_PI_2, viewportSize.x / viewportSize.y);
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

glm::mat4 evan::DesktopSwapchainContext::getProjection(std::size_t index) const
{
	return _view.getProjectionMatrix();
}

utility::graphic::ViewF
	evan::DesktopSwapchainContext::getView(std::size_t index) const
{
	return _view;
}

void evan::DesktopSwapchainContext::setView(std::size_t index,
											const utility::graphic::ViewF &view)
{
	_view = view;
}

std::size_t evan::DesktopSwapchainContext::getViewCount(void) const
{
	return 1;
}
