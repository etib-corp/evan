/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrSwapchainContext
*/

#include "evan/openxr/XrSwapchainContext.hpp"

#include "evan/CheckedCast.hpp"

#include <algorithm>

#include <glm/gtc/quaternion.hpp>

namespace
{
	/**
	 * @brief Converts an OpenXR sample count to its Vulkan flag bit.
	 *
	 * OpenXR exposes swapchain sample counts as plain uint32_t values while
	 * Vulkan uses VkSampleCountFlagBits. Unsupported or zero counts map to
	 * VK_SAMPLE_COUNT_1_BIT.
	 */
	VkSampleCountFlagBits toVkSampleCountFlagBits(uint32_t sampleCount)
	{
		switch (sampleCount) {
			case 2:
				return VK_SAMPLE_COUNT_2_BIT;
			case 4:
				return VK_SAMPLE_COUNT_4_BIT;
			case 8:
				return VK_SAMPLE_COUNT_8_BIT;
			case 16:
				return VK_SAMPLE_COUNT_16_BIT;
			case 32:
				return VK_SAMPLE_COUNT_32_BIT;
			case 64:
				return VK_SAMPLE_COUNT_64_BIT;
			default:
				return VK_SAMPLE_COUNT_1_BIT;
		}
	}
}	 // namespace

evan::XrSwapchainContext::XrSwapchainContext(const DeviceContext &deviceContext)
{
	this->getLogger().info() << "Initializing XrSwapchainContext";

	auto &backend = evan::checkedCast<evan::XrDeviceBackend>(
		*deviceContext.getDeviceBackend());

	uint32_t swapchainFormatCount =
		deviceContext.getDeviceBackend()->countSwapchainFormats();
	auto swapchainFormats =
		deviceContext.getDeviceBackend()->enumerateSwapchainFormats(
			swapchainFormatCount);

	_viewsConfigurations = backend.enumerateViewConfigurations();
	_views.resize(_viewsConfigurations.size(), { XR_TYPE_VIEW });

	_viewSet.resize(_viewsConfigurations.size());
	for (std::size_t i = 0; i < _viewsConfigurations.size(); ++i) {
		_viewSet[i].swapchainIndex = i;
	}

	selectMsaaSamples(deviceContext);

	createRenderPass(deviceContext.getDeviceBackend(), _msaaSamples,
					 _colorAttachmentMode);

	auto swapchainFormat = selectSwapchainFormat(
		deviceContext.getDeviceBackend()->getPhysicalDevice(),
		swapchainFormats);

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
		// The render pass and framebuffers are built for this sample count
		// (see selectMsaaSamples), so it must stay the runtime's recommended
		// value rather than the application one.
		swapchainCreateInfo.sampleCount =
			viewConfig.recommendedSwapchainSampleCount;
		swapchainCreateInfo.usageFlags =
			backend.getOpenXrOptions().swapchainUsageFlags;

		XrSwapchain swapchain;
		auto session = backend.getSession();
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
			.swapchain			 = swapchain,
			.createInfo			 = swapchainCreateInfo,
			.renderPass			 = _renderPass,
			.deviceContext		 = deviceContext,
			.msaaSamples		 = _msaaSamples,
			.colorAttachmentMode = _colorAttachmentMode
		};
		_swapchainImages.push_back(
			std::make_shared<XrSwapchainImage>(properties));
	}
}

void evan::XrSwapchainContext::selectMsaaSamples(
	const DeviceContext &deviceContext)
{
	// The swapchain images belong to the runtime, and their sample count is
	// whatever it handed out (XrSwapchainCreateInfo::sampleCount asks for the
	// recommended value). The render pass and the framebuffers have to match
	// that count, so a runtime that only offers multisampled swapchain images
	// overrides the application setting.
	const uint32_t recommended = _viewsConfigurations.empty()
		? 0u
		: _viewsConfigurations.front().recommendedSwapchainSampleCount;
	const VkSampleCountFlagBits runtimeSamples =
		toVkSampleCountFlagBits(recommended);
	const VkSampleCountFlagBits requested = deviceContext.getMsaaSamples();

	if (_viewsConfigurations.empty()) {
		this->getLogger().warning()
			<< "No OpenXR view configuration available. Using " << requested
			<< " sample(s).";
		_msaaSamples		 = requested;
		_colorAttachmentMode = requested == VK_SAMPLE_COUNT_1_BIT
			? ColorAttachmentMode::SwapchainImageAttachment
			: ColorAttachmentMode::ResolveToSwapchain;
		return;
	}

	if (runtimeSamples != VK_SAMPLE_COUNT_1_BIT) {
		// The runtime only hands out multisampled swapchain images: render
		// into them directly, their sample count cannot be lowered.
		_msaaSamples		 = runtimeSamples;
		_colorAttachmentMode = ColorAttachmentMode::SwapchainImageAttachment;
		if (requested != runtimeSamples) {
			this->getLogger().warning()
				<< "The OpenXR runtime recommends a " << recommended
				<< "-sample swapchain, so the application request of "
				<< requested << " sample(s) is ignored.";
		}
	} else if (requested == VK_SAMPLE_COUNT_1_BIT) {
		// Default path: no multisampling anywhere and no resolve. The render
		// pass targets the single-sampled swapchain image directly.
		_msaaSamples		 = VK_SAMPLE_COUNT_1_BIT;
		_colorAttachmentMode = ColorAttachmentMode::SwapchainImageAttachment;
	} else {
		// The application asked for multisampling: shade into an engine-owned
		// multisampled color image and resolve it into the single-sampled
		// swapchain image.
		_msaaSamples		 = requested;
		_colorAttachmentMode = ColorAttachmentMode::ResolveToSwapchain;
	}

	this->getLogger().info()
		<< "OpenXR recommended swapchain sample count: " << recommended
		<< ", using MSAA samples: " << _msaaSamples
		<< ", resolve to swapchain: "
		<< (_colorAttachmentMode == ColorAttachmentMode::ResolveToSwapchain
				? "yes"
				: "no");
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

	auto &backend = evan::checkedCast<evan::XrDeviceBackend>(
		*deviceContext.getDeviceBackend());

	for (const auto &swapchainImage: _swapchainImages) {
		this->getLogger().info()
			<< "Destroying swapchain image and releasing resources";
		swapchainImage->destroy(deviceContext.getDeviceBackend()->getDevice());
	}
	_swapchainImages.clear();

	for (const auto &viewConfig: _viewsConfigurations) {
		this->getLogger().info() << "Creating swapchain for view configuration";

		XrSwapchainCreateInfo swapchainCreateInfo {};
		swapchainCreateInfo.type	  = XR_TYPE_SWAPCHAIN_CREATE_INFO;
		swapchainCreateInfo.arraySize = 1;
		swapchainCreateInfo.format	  = selectSwapchainFormat(
			   deviceContext.getDeviceBackend()->getPhysicalDevice(),
			   deviceContext.getDeviceBackend()->enumerateSwapchainFormats(
				   deviceContext.getDeviceBackend()->countSwapchainFormats()));
		swapchainCreateInfo.width	  = viewConfig.recommendedImageRectWidth;
		swapchainCreateInfo.height	  = viewConfig.recommendedImageRectHeight;
		swapchainCreateInfo.mipCount  = 1;
		swapchainCreateInfo.faceCount = 1;
		// The render pass and framebuffers are built for this sample count
		// (see selectMsaaSamples), so it must stay the runtime's recommended
		// value rather than the application one.
		swapchainCreateInfo.sampleCount =
			viewConfig.recommendedSwapchainSampleCount;
		swapchainCreateInfo.usageFlags =
			backend.getOpenXrOptions().swapchainUsageFlags;

		XrSwapchain swapchain;
		auto session = backend.getSession();
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
			.swapchain			 = swapchain,
			.createInfo			 = swapchainCreateInfo,
			.renderPass			 = _renderPass,
			.deviceContext		 = deviceContext,
			.msaaSamples		 = _msaaSamples,
			.colorAttachmentMode = _colorAttachmentMode
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
		evan::checkedCast<XrSwapchainImage>(*_swapchainImages[index])
			._swapchain;
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
		evan::checkedCast<XrSwapchainImage>(*_swapchainImages[index])
			._swapchain;
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

VkSampleCountFlagBits evan::XrSwapchainContext::getMsaaSamples() const
{
	return _msaaSamples;
}

void evan::XrSwapchainContext::syncViewSet()
{
	this->getLogger().info() << "Syncing ViewSet from OpenXR view state";

	if (_viewSet.size() != _views.size()) {
		_viewSet.resize(_views.size());
	}

	// Track the raw head center before applying the locomotion offset so that
	// setView can reduce the requested pose to an offset relative to it.
	if (!_views.empty()) {
		glm::vec3 centerPosition(0.0f);
		for (const auto &xrView: _views) {
			centerPosition +=
				glm::vec3(xrView.pose.position.x, xrView.pose.position.y,
						  xrView.pose.position.z);
		}
		centerPosition /= static_cast<float>(_views.size());

		glm::quat centerOrientation(1.0f, 0.0f, 0.0f, 0.0f);
		if (_views.size() == 1) {
			const auto &orientation = _views[0].pose.orientation;
			centerOrientation		= glm::quat(orientation.w, orientation.x,
												orientation.y, orientation.z);
		} else {
			const auto &left  = _views[0].pose.orientation;
			const auto &right = _views[1].pose.orientation;
			glm::quat leftQ(left.w, left.x, left.y, left.z);
			glm::quat rightQ(right.w, right.x, right.y, right.z);
			centerOrientation = glm::normalize(glm::slerp(leftQ, rightQ, 0.5f));
		}

		_baseCenterPose = utility::graphic::PoseF(
			utility::graphic::PositionF(centerPosition.x, centerPosition.y,
										centerPosition.z),
			utility::graphic::OrientationF(
				centerOrientation.x, centerOrientation.y, centerOrientation.z,
				centerOrientation.w));
		_hasBaseCenter = true;
	}

	const auto &offsetPose	 = _viewOffset.getPosition();
	const auto &offsetOrient = _viewOffset.getOrientation();
	const glm::quat offsetQ(offsetOrient.w, offsetOrient.x, offsetOrient.y,
							offsetOrient.z);
	const glm::vec3 offsetPosition(offsetPose.getX(), offsetPose.getY(),
								   offsetPose.getZ());

	for (std::size_t i = 0; i < _views.size(); ++i) {
		_viewSet[i].swapchainIndex = i;

		utility::graphic::ViewF view;
		const auto &xrPose = _views[i].pose;

		glm::quat baseQ(xrPose.orientation.w, xrPose.orientation.x,
						xrPose.orientation.y, xrPose.orientation.z);
		glm::quat finalQ = glm::normalize(offsetQ * baseQ);

		glm::vec3 basePosition(xrPose.position.x, xrPose.position.y,
							   xrPose.position.z);
		glm::vec3 finalPosition = basePosition + offsetPosition;

		utility::graphic::PoseF pose(
			utility::graphic::PositionF(finalPosition.x, finalPosition.y,
										finalPosition.z),
			utility::graphic::OrientationF(finalQ.x, finalQ.y, finalQ.z,
										   finalQ.w));
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
	(void)index;

	_nearPlane = view.getNearPlane();
	_farPlane  = view.getFarPlane();

	if (!_hasBaseCenter) {
		return;
	}

	// The runtime refreshes the tracked eye poses every frame, so the requested
	// pose is stored as a world-space offset relative to the tracked head
	// center and re-applied by syncViewSet.
	const auto requestedPosition = view.getPose().getPosition();
	const auto requestedOrient	 = view.getPose().getOrientation();
	const auto basePosition		 = _baseCenterPose.getPosition();
	const auto baseOrient		 = _baseCenterPose.getOrientation();

	glm::quat requestedQ(requestedOrient.w, requestedOrient.x,
						 requestedOrient.y, requestedOrient.z);
	glm::quat baseQ(baseOrient.w, baseOrient.x, baseOrient.y, baseOrient.z);

	glm::quat offsetQ = glm::normalize(requestedQ * glm::inverse(baseQ));

	_viewOffset = utility::graphic::PoseF(
		utility::graphic::PositionF(
			requestedPosition.getX() - basePosition.getX(),
			requestedPosition.getY() - basePosition.getY(),
			requestedPosition.getZ() - basePosition.getZ()),
		utility::graphic::OrientationF(offsetQ.x, offsetQ.y, offsetQ.z,
									   offsetQ.w));
}
