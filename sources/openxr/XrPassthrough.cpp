/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrPassthrough
*/

#include "evan/openxr/XrPassthrough.hpp"
#include "evan/openxr/XrError.hpp"

evan::XrPassthrough::XrPassthrough(
	XrSession session, const XrInstanceFunctions::Passthrough &functions)
	: _session(session)
	, _functions(functions)
{
}

evan::XrPassthrough::~XrPassthrough()
{
	(void)this->stop();
}

bool evan::XrPassthrough::isSupported() const noexcept
{
	return _session != XR_NULL_SESSION && _functions.isComplete();
}

evan::Error evan::XrPassthrough::start()
{
	if (_running) {
		return Error::Ok;
	}
	if (!isSupported()) {
		this->getLogger().warning()
			<< "Passthrough is not supported on this runtime.";
		return Error::Unsupported;
	}

	if (_passthrough == XR_NULL_HANDLE) {
		XrPassthroughCreateInfoFB createInfo {
			XR_TYPE_PASSTHROUGH_CREATE_INFO_FB
		};
		createInfo.flags = 0;

		const XrResult result =
			_functions.create(_session, &createInfo, &_passthrough);
		if (result != XR_SUCCESS) {
			this->getLogger().error()
				<< "Failed to create passthrough: " << result;
			return mapXrResult(result);
		}
	}

	if (_layer == XR_NULL_HANDLE) {
		XrPassthroughLayerCreateInfoFB layerCreateInfo {
			XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_FB
		};
		layerCreateInfo.passthrough = _passthrough;
		layerCreateInfo.flags		= 0;
		layerCreateInfo.purpose =
			XR_PASSTHROUGH_LAYER_PURPOSE_RECONSTRUCTION_FB;

		const XrResult result =
			_functions.createLayer(_session, &layerCreateInfo, &_layer);
		if (result != XR_SUCCESS) {
			this->getLogger().error()
				<< "Failed to create passthrough layer: " << result;
			(void)this->stop();
			return mapXrResult(result);
		}
	}

	XrResult result = _functions.start(_passthrough);
	if (result != XR_SUCCESS) {
		this->getLogger().error() << "Failed to start passthrough: " << result;
		(void)this->stop();
		return mapXrResult(result);
	}

	result = _functions.resumeLayer(_layer);
	if (result != XR_SUCCESS) {
		this->getLogger().error()
			<< "Failed to resume passthrough layer: " << result;
		(void)this->stop();
		return mapXrResult(result);
	}

	_running = true;
	return Error::Ok;
}

evan::Error evan::XrPassthrough::stop()
{
	XrResult firstError = XR_SUCCESS;

	if (_layer != XR_NULL_HANDLE) {
		XrResult result = _functions.pauseLayer(_layer);
		if (result != XR_SUCCESS && firstError == XR_SUCCESS) {
			firstError = result;
		}
		result = _functions.destroyLayer(_layer);
		if (result != XR_SUCCESS && firstError == XR_SUCCESS) {
			firstError = result;
		}
		_layer = XR_NULL_HANDLE;
	}

	if (_passthrough != XR_NULL_HANDLE) {
		XrResult result = _functions.pause(_passthrough);
		if (result != XR_SUCCESS && firstError == XR_SUCCESS) {
			firstError = result;
		}
		result = _functions.destroy(_passthrough);
		if (result != XR_SUCCESS && firstError == XR_SUCCESS) {
			firstError = result;
		}
		_passthrough = XR_NULL_HANDLE;
	}

	_running = false;

	if (firstError != XR_SUCCESS) {
		this->getLogger().error()
			<< "Failed to stop passthrough: " << firstError;
		return mapXrResult(firstError);
	}
	return Error::Ok;
}

bool evan::XrPassthrough::isRunning() const noexcept
{
	return _running;
}

XrPassthroughLayerFB evan::XrPassthrough::getLayer() const noexcept
{
	return _layer;
}
