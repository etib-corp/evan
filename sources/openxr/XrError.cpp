/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrError
*/

#include "evan/openxr/XrError.hpp"

evan::Error evan::mapXrResult(XrResult result)
{
	switch (result) {
		case XR_SUCCESS:
			return Error::Ok;
		case XR_ERROR_SESSION_LOST:
		case XR_ERROR_INSTANCE_LOST:
			return Error::RuntimeLost;
		case XR_ERROR_SESSION_NOT_RUNNING:
		case XR_ERROR_TIME_INVALID:
			return Error::NotReady;
		case XR_ERROR_OUT_OF_MEMORY:
			return Error::OutOfMemory;
		default:
			return Error::RuntimeError;
	}
}

evan::Error evan::mapSessionState(XrSessionState state)
{
	switch (state) {
		case XR_SESSION_STATE_LOSS_PENDING:
			return Error::RuntimeLost;
		default:
			return Error::Ok;
	}
}
