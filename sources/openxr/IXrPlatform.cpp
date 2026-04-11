/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrPlatform
*/

#include "openxr/IXrPlatform.hpp"
#include "openxr/XrDeviceBackend.hpp"

////////////////////
// Public Methods //
////////////////////

bool evan::IXrPlatform::shouldClose() const
{
	return _shouldClose;
}

void evan::IXrPlatform::pollEvents(ADeviceBackend &deviceBackend)
{
	XrEventDataBuffer eventDataBuffer { XR_TYPE_EVENT_DATA_BUFFER };
	evan::XrDeviceBackend &xrDeviceBackend =
		dynamic_cast<evan::XrDeviceBackend &>(deviceBackend);

	xrDeviceBackend.pollActions();

	while (xrPollEvent(xrDeviceBackend._XrInstance, &eventDataBuffer) == XR_SUCCESS) {
		switch (eventDataBuffer.type) {
			case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
				auto sessionStateChangedEvent =
					*reinterpret_cast<XrEventDataSessionStateChanged *>(
						&eventDataBuffer);
				processSessionStateChangedEvent(sessionStateChangedEvent,
												xrDeviceBackend);
				break;
			}
			case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
				_shouldClose = true;
				break;
			}
			default:
				break;
		}
		eventDataBuffer = { XR_TYPE_EVENT_DATA_BUFFER };
	}
}

std::shared_ptr<evan::ADeviceBackend> evan::IXrPlatform::createDeviceBackend() const
{
	return std::make_shared<XrDeviceBackend>(*this);
}

std::shared_ptr<evan::ASwapchainContext>evan::IXrPlatform::createSwapchainContext(
	const DeviceContext &deviceContext) const
{
	return std::make_shared<XrSwapchainContext>(deviceContext);
}

///////////////////////
// Protected Methods //
///////////////////////

void evan::IXrPlatform::processSessionStateChangedEvent(
	const XrEventDataSessionStateChanged &eventData, evan::XrDeviceBackend &xrDeviceBackend)
{
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
