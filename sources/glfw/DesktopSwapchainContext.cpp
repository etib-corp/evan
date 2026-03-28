/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopSwapchainContext
*/

#include "glfw/DesktopSwapchainContext.hpp"

#include "DeviceContext.hpp"

#include <utility/math/vector.hpp>
#include <utility/math/trigonometric.hpp>

evan::DesktopSwapchainContext::DesktopSwapchainContext(
	const DeviceContext &deviceContext, GLFWwindow *window)
{
	this->createRenderPass(deviceContext.getDeviceBackend(),
						   deviceContext.getMsaaSamples());
	_swapchainImages.push_back(std::make_shared<DesktopSwapchainImage>(
		deviceContext, window, _renderPass));
}

evan::DesktopSwapchainContext::~DesktopSwapchainContext()
{
}

////////////////////
// Public Methods //
////////////////////

void evan::DesktopSwapchainContext::destroy(VkDevice device)
{
	vkDestroyRenderPass(device, _renderPass, nullptr);
	for (const auto &swapchainImage: _swapchainImages) {
		swapchainImage->destroy(device);
	}
}

VkResult evan::DesktopSwapchainContext::aquireImage(
	uint32_t index, VkDevice device, VkSemaphore imageAvailableSemaphore,
	VkFence inFlightFence, uint32_t &imageIndex)
{
	VkSwapchainKHR swapchain =
		dynamic_cast<DesktopSwapchainImage *>(_swapchainImages[index].get())
			->_swapchain;

	return vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
								 imageAvailableSemaphore, inFlightFence,
								 &imageIndex);
}

utility::math::Matrix4x4F
	evan::DesktopSwapchainContext::getProjection(int index) const
{
	return utility::math::Matrix4x4F(utility::math::perspective(
		utility::math::radians(45.0f), 800.0f / 600.0f, 0.1f, 10.0f));
}

utility::math::Matrix4x4F
	evan::DesktopSwapchainContext::getView(int index) const
{
	return utility::math::Matrix4x4F(
		utility::math::lookAt(utility::math::Vector3F({ 0.0f, 0.0f, 2.0f }),
							  utility::math::Vector3F({ 0.0f, 0.0f, 0.0f }),
							  utility::math::Vector3F({ 0.0f, 1.0f, 0.0f })));
	utility::math::Vector3F({ 0.0f, 1.0f, 0.0f });
}