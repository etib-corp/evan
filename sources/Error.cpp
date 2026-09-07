/*
** ETIB PROJECT, 2026
** evan
** File description:
** Error
*/

#include "evan/Error.hpp"

evan::Error evan::mapVkResult(VkResult result)
{
	switch (result) {
		case VK_SUCCESS:
			return Error::Ok;
		case VK_SUBOPTIMAL_KHR:
			return Error::Suboptimal;
		case VK_ERROR_OUT_OF_DATE_KHR:
		case VK_ERROR_SURFACE_LOST_KHR:
			return Error::SwapchainOutOfDate;
		case VK_NOT_READY:
		case VK_TIMEOUT:
			return Error::NotReady;
		case VK_ERROR_DEVICE_LOST:
			return Error::DeviceLost;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return Error::OutOfMemory;
		default:
			return Error::RuntimeError;
	}
}
