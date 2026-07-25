/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrPlatform
*/

#include "evan/openxr/IXrPlatform.hpp"
#include "evan/openxr/XrDeviceBackend.hpp"

////////////////////
// Public Methods //
////////////////////

bool evan::IXrPlatform::shouldClose() const
{
	return _shouldClose;
}

std::vector<std::shared_ptr<utility::event::Event>>
	evan::IXrPlatform::pollEvents(ADeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Polling OpenXR events";

	XrEventDataBuffer eventDataBuffer { XR_TYPE_EVENT_DATA_BUFFER };
	evan::XrDeviceBackend &xrDeviceBackend =
		dynamic_cast<evan::XrDeviceBackend &>(deviceBackend);

	std::vector<std::shared_ptr<utility::event::Event>> events =
		xrDeviceBackend.pollActions();

	while (xrPollEvent(xrDeviceBackend._XrInstance, &eventDataBuffer)
		   == XR_SUCCESS) {
		switch (eventDataBuffer.type) {
			case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
				this->getLogger().info()
					<< "Processing session state changed event";
				auto sessionStateChangedEvent =
					*reinterpret_cast<XrEventDataSessionStateChanged *>(
						&eventDataBuffer);
				processSessionStateChangedEvent(sessionStateChangedEvent,
												xrDeviceBackend);
				break;
			}
			case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
				this->getLogger().info()
					<< "Instance loss pending event received";
				_shouldClose = true;
				break;
			}
			default:
				this->getLogger().warning()
					<< "Unhandled event type: " << eventDataBuffer.type;
				break;
		}
		eventDataBuffer = { XR_TYPE_EVENT_DATA_BUFFER };
	}
	return events;
}

std::shared_ptr<evan::ADeviceBackend>
	evan::IXrPlatform::createDeviceBackend() const
{
	return std::make_shared<XrDeviceBackend>(*this);
}

std::shared_ptr<evan::ASwapchainContext>
	evan::IXrPlatform::createSwapchainContext(
		const DeviceContext &deviceContext) const
{
	auto swapchainContext =

		std::make_shared<XrSwapchainContext>(deviceContext);

	utility::graphic::ViewF view;
	view.setClippingPlanes(1.0f, 4000.0f);

	this->getLogger().error()
		<< "Created swapchain context with view: " << view;

	swapchainContext->setView(0, view);
	swapchainContext->setView(1, view);

	return swapchainContext;
}

///////////////////////
// Protected Methods //
///////////////////////

void evan::IXrPlatform::processSessionStateChangedEvent(
	const XrEventDataSessionStateChanged &eventData,
	evan::XrDeviceBackend &xrDeviceBackend)
{
	this->getLogger().info() << "Processing session state changed event";

	switch (eventData.state) {
		case XR_SESSION_STATE_READY: {
			XrSessionBeginInfo sessionBeginInfo;
			sessionBeginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
			sessionBeginInfo.primaryViewConfigurationType =
				XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
			xrBeginSession(xrDeviceBackend._session, &sessionBeginInfo);
			xrDeviceBackend._sessionRunning = true;
			break;
		}
		case XR_SESSION_STATE_STOPPING:
			xrEndSession(xrDeviceBackend._session);
			xrDeviceBackend._sessionRunning = false;
			break;
		case XR_SESSION_STATE_EXITING:
			_shouldClose = true;
			break;
		case XR_SESSION_STATE_LOSS_PENDING:
			_shouldClose = true;
			break;
		default:
			break;
	}
}
