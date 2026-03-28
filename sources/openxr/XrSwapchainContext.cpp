/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrSwapchainContext
*/

#include "openxr/XrSwapchainContext.hpp"

#include <utility/math/matrix.hpp>
#include <utility/math/vector.hpp>
#include <utility/math/quaternion.hpp>

#include <algorithm>

evan::XrSwapchainContext::XrSwapchainContext(const DeviceContext &deviceContext)
{
	createRenderPass(deviceContext.getDeviceBackend(),
					 deviceContext.getMsaaSamples());

	uint32_t swapchainFormatCount =
		deviceContext.getDeviceBackend()->countSwapchainFormats();
	auto swapchainFormats =
		deviceContext.getDeviceBackend()->enumerateSwapchainFormats(
			swapchainFormatCount);

	_viewsConfigurations =
		dynamic_cast<XrDeviceBackend *>(deviceContext.getDeviceBackend().get())
			->enumerateViewConfigurations();
	_views.resize(_viewsConfigurations.size(), { XR_TYPE_VIEW });

	auto swapchainFormat = selectSwapchainFormat(swapchainFormats);

	for (const auto &viewConfig: _viewsConfigurations) {
		XrSwapchainCreateInfo swapchainCreateInfo {};
		swapchainCreateInfo.type	  = XR_TYPE_SWAPCHAIN_CREATE_INFO;
		swapchainCreateInfo.arraySize = 1;
		swapchainCreateInfo.format	  = swapchainFormat;
		swapchainCreateInfo.width	  = viewConfig.recommendedImageRectWidth;
		swapchainCreateInfo.height	  = viewConfig.recommendedImageRectHeight;
		swapchainCreateInfo.mipCount  = 1;
		swapchainCreateInfo.faceCount = 1;
		swapchainCreateInfo.sampleCount =
			viewConfig.recommendedSwapchainSampleCount;
		swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT
			| XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;

		XrSwapchain swapchain;
		auto session = dynamic_cast<evan::XrDeviceBackend *>(
						   deviceContext.getDeviceBackend().get())
						   ->_session;
		XrResult result =
			xrCreateSwapchain(session, &swapchainCreateInfo, &swapchain);
		if (result != XR_SUCCESS) {
			std::cerr << "Failed to create swapchain for view configuration "
						 "with error code: "
					  << result << std::endl;
			continue;
		}
		evan::XrSwapchainImage::CreateXrSwapchainImageProperties properties {
			.swapchain	   = swapchain,
			.createInfo	   = swapchainCreateInfo,
			.renderPass	   = _renderPass,
			.deviceContext = deviceContext
		};
		_swapchainImages.push_back(
			std::make_shared<XrSwapchainImage>(properties));
	}
}

////////////////////
// Public Methods //
////////////////////

void evan::XrSwapchainContext::destroy(VkDevice device)
{
	for (const auto &swapchainImage: _swapchainImages) {
		swapchainImage->destroy(device);
	}
	_swapchainImages.clear();
}

VkResult evan::XrSwapchainContext::aquireImage(
	uint32_t index, VkDevice device, VkSemaphore imageAvailableSemaphore,
	VkFence inFlightFence, uint32_t &imageIndex)
{
	XrSwapchain swapchain =
		dynamic_cast<XrSwapchainImage *>(_swapchainImages[index].get())
			->_swapchain;
	XrSwapchainImageAcquireInfo acquire_info {};
	acquire_info.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
	XrResult result =
		xrAcquireSwapchainImage(swapchain, &acquire_info, &imageIndex);
	if (result != XR_SUCCESS) {
		std::cerr << "Failed to acquire swapchain image with error code: "
				  << result << std::endl;
		return VK_ERROR_OUT_OF_DATE_KHR;
	}
	return VK_SUCCESS;
}

void evan::XrSwapchainContext::waitForImage(uint32_t index)
{
	XrSwapchain swapchain =
		dynamic_cast<XrSwapchainImage *>(_swapchainImages[index].get())
			->_swapchain;
	XrSwapchainImageWaitInfo wait_info {};
	wait_info.type	  = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
	wait_info.timeout = XR_INFINITE_DURATION;
	XrResult result	  = xrWaitSwapchainImage(swapchain, &wait_info);
	if (result != XR_SUCCESS) {
		std::cerr << "Failed to wait for swapchain image with error code: "
				  << result << std::endl;
	}
}

void evan::XrSwapchainContext::updateProjectionLayerViews()
{
	const size_t count = std::min(_views.size(), _swapchainImages.size());

	_projectionLayerViews.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto *viewSwapchain =
			dynamic_cast<XrSwapchainImage *>(_swapchainImages[i].get());

		if (viewSwapchain == nullptr) {
			continue;
		}

		_projectionLayerViews[i] = {};
		_projectionLayerViews[i].type =
			XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
		_projectionLayerViews[i].pose				= _views[i].pose;
		_projectionLayerViews[i].fov				= _views[i].fov;
		_projectionLayerViews[i].subImage.swapchain = viewSwapchain->_swapchain;
		_projectionLayerViews[i].subImage.imageRect.offset = { 0, 0 };
		_projectionLayerViews[i].subImage.imageRect.extent = {
			static_cast<int32_t>(viewSwapchain->_width),
			static_cast<int32_t>(viewSwapchain->_height)
		};
	}
}

/////////////
// Getters //
/////////////

const std::vector<XrCompositionLayerProjectionView> &
	evan::XrSwapchainContext::getProjectionLayerViews() const
{
	return _projectionLayerViews;
}

utility::math::Matrix4x4F
	evan::XrSwapchainContext::getProjection(int index) const
{
	const XrFovf &fov = _views[index].fov;
	float nearZ		  = 0.1f;
	float farZ		  = 10.0f;

	float tanLeft  = tanf(fov.angleLeft);
	float tanRight = tanf(fov.angleRight);
	float tanUp	   = tanf(fov.angleUp);
	float tanDown  = tanf(fov.angleDown);

	float width	 = tanRight - tanLeft;
	float height = tanUp - tanDown;

	utility::math::Matrix4x4F projection(0.0f);
	projection[0][0] = 2.0f / width;
	projection[1][1] = 2.0f / height;
	projection[2][0] = (tanRight + tanLeft) / width;
	projection[2][1] = (tanUp + tanDown) / height;
	projection[2][2] = -farZ / (farZ - nearZ);
	projection[2][3] = -1.0f;
	projection[3][2] = -(farZ * nearZ) / (farZ - nearZ);

	return projection;
}

utility::math::Matrix4x4F evan::XrSwapchainContext::getView(int index) const
{
	const auto &pose = _views[index].pose;
	const utility::math::QuaternionF orientation(
		pose.orientation.w, pose.orientation.x, pose.orientation.y,
		pose.orientation.z);
	const utility::math::Vector3F position(
		{ pose.position.x, pose.position.y, pose.position.z });

	const utility::math::Matrix4x4F rotation =
		utility::math::Matrix4x4F(utility::math::conjugate(orientation));
	const utility::math::Matrix4x4F translation = utility::math::translate(
		utility::math::Matrix4x4F({1.0f, 0.0f, 0.0f, 0.0f}), -position);
	return rotation * translation;
}
