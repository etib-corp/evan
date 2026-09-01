/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrSwapchainContext
*/

#include "evan/openxr/XrSwapchainContext.hpp"

#include <algorithm>

evan::XrSwapchainContext::XrSwapchainContext(const DeviceContext &deviceContext)
{
	this->getLogger().info() << "Initializing XrSwapchainContext";

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

	_viewSet.resize(_viewsConfigurations.size());
	for (std::size_t i = 0; i < _viewsConfigurations.size(); ++i) {
		_viewSet[i].swapchainIndex = i;
	}

	auto swapchainFormat = selectSwapchainFormat(swapchainFormats);

	for (const auto &viewConfig: _viewsConfigurations) {
		this->getLogger().info() << "Creating swapchain for view configuration";

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
			this->getLogger().error()
				<< "Failed to create swapchain for view configuration "
				   "with error code: "
				<< result;
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
	this->getLogger().info()
		<< "Destroying XrSwapchainContext and releasing resources";

	for (const auto &swapchainImage: _swapchainImages) {
		this->getLogger().info()
			<< "Destroying swapchain image and releasing resources";
		swapchainImage->destroy(device);
	}
	_swapchainImages.clear();
}

void evan::XrSwapchainContext::recreateSwapchain(
	const DeviceContext &deviceContext, VkRenderPass renderpass)
{
	this->getLogger().info() << "Recreating swapchain and associated resources "
								"for XrSwapchainContext";

	for (const auto &swapchainImage: _swapchainImages) {
		this->getLogger().info()
			<< "Destroying swapchain image and releasing resources";
		swapchainImage->destroy(deviceContext.getDeviceBackend()->_device);
	}
	_swapchainImages.clear();

	for (const auto &viewConfig: _viewsConfigurations) {
		this->getLogger().info() << "Creating swapchain for view configuration";

		XrSwapchainCreateInfo swapchainCreateInfo {};
		swapchainCreateInfo.type	  = XR_TYPE_SWAPCHAIN_CREATE_INFO;
		swapchainCreateInfo.arraySize = 1;
		swapchainCreateInfo.format	  = selectSwapchainFormat(
			deviceContext.getDeviceBackend()->enumerateSwapchainFormats(
				deviceContext.getDeviceBackend()->countSwapchainFormats()));
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
			this->getLogger().error()
				<< "Failed to create swapchain for view configuration "
				   "with error code: "
				<< result;
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
	this->getLogger().info() << "Finished recreating swapchain and associated "
								"resources for XrSwapchainContext";
}

VkResult evan::XrSwapchainContext::aquireImage(
	uint32_t index, VkDevice device, VkSemaphore imageAvailableSemaphore,
	VkFence inFlightFence, uint32_t &imageIndex)
{
	this->getLogger().info() << "Acquiring swapchain image";

	XrSwapchain swapchain =
		dynamic_cast<XrSwapchainImage *>(_swapchainImages[index].get())
			->_swapchain;
	XrSwapchainImageAcquireInfo acquire_info {};
	acquire_info.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
	XrResult result =
		xrAcquireSwapchainImage(swapchain, &acquire_info, &imageIndex);
	if (result != XR_SUCCESS) {
		this->getLogger().error()
			<< "Failed to acquire swapchain image with error code: " << result;
		return VK_ERROR_OUT_OF_DATE_KHR;
	}
	return VK_SUCCESS;
}

void evan::XrSwapchainContext::waitForImage(uint32_t index)
{
	this->getLogger().info() << "Waiting for swapchain image";

	XrSwapchain swapchain =
		dynamic_cast<XrSwapchainImage *>(_swapchainImages[index].get())
			->_swapchain;
	XrSwapchainImageWaitInfo wait_info {};
	wait_info.type	  = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
	wait_info.timeout = XR_INFINITE_DURATION;
	XrResult result	  = xrWaitSwapchainImage(swapchain, &wait_info);
	if (result != XR_SUCCESS) {
		this->getLogger().error()
			<< "Failed to wait for swapchain image with error code: " << result;
		return;
	}
	this->getLogger().info() << "Successfully waited for swapchain image";
}

void evan::XrSwapchainContext::updateProjectionLayerViews()
{
	this->getLogger().info()
		<< "Updating projection layer views based on current swapchain images "
		   "and view configurations";

	const size_t count = std::min(_views.size(), _swapchainImages.size());

	_projectionLayerViews.resize(count);
	for (size_t i = 0; i < count; ++i) {
		this->getLogger().info()
			<< "Updating projection layer view for index: " << i;
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
	this->getLogger().info() << "Finished updating projection layer views";
}

/////////////
// Getters //
/////////////

const std::vector<XrCompositionLayerProjectionView> &
	evan::XrSwapchainContext::getProjectionLayerViews() const
{
	return _projectionLayerViews;
}

evan::ViewSet &evan::XrSwapchainContext::getViewSet()
{
	return _viewSet;
}

const evan::ViewSet &evan::XrSwapchainContext::getViewSet() const
{
	return _viewSet;
}

void evan::XrSwapchainContext::syncViewSet()
{
	this->getLogger().info() << "Syncing ViewSet from OpenXR view state";

	if (_viewSet.size() != _views.size()) {
		_viewSet.resize(_views.size());
	}

	for (std::size_t i = 0; i < _views.size(); ++i) {
		_viewSet[i].swapchainIndex = i;

		utility::graphic::ViewF view;
		const auto &xrPose = _views[i].pose;
		utility::graphic::PositionF position(
			xrPose.position.x, xrPose.position.y, xrPose.position.z);
		utility::graphic::OrientationF orientation(
			xrPose.orientation.x, xrPose.orientation.y, xrPose.orientation.z,
			xrPose.orientation.w);
		utility::graphic::PoseF pose(position, orientation);
		utility::graphic::FieldOfViewF fov(
			_views[i].fov.angleUp, _views[i].fov.angleDown,
			_views[i].fov.angleLeft, _views[i].fov.angleRight);

		view.setPose(pose);
		view.setFieldOfView(fov);
		view.setClippingPlanes(_nearPlane, _farPlane);

		_viewSet[i].view = view;
	}
}

void evan::XrSwapchainContext::setView(std::size_t index,
									   const utility::graphic::ViewF &view)

{
	_nearPlane = view.getNearPlane();
	_farPlane  = view.getFarPlane();
}
