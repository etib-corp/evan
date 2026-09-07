/*
** ETIB PROJECT, 2026
** evan
** File description:
** Error
*/

#pragma once

#include "evan/EvanPlatform.hpp"

namespace evan
{
	/**
	 * @brief Normalized error taxonomy shared by every Evan subsystem.
	 *
	 * Backends map their native result codes (VkResult, XrResult, GLFW) into
	 * this enum at the boundary so that consumers can react uniformly:
	 * recoverable errors can be handled (e.g. recreate the swapchain), fatal
	 * errors mean the application should tear down cleanly.
	 *
	 * @see mapVkResult
	 * @see isFatal
	 */
	enum class Error {
		Ok = 0,

		// Recoverable — the consumer should react and keep running.
		Suboptimal,			 // VK_SUBOPTIMAL_KHR: presentable, recreate soon
		SwapchainOutOfDate,	 // VK_ERROR_OUT_OF_DATE_KHR: recreate swapchain
		NotReady,			 // VK_NOT_READY / XR session not running / invalid
							 // time

		// Fatal — the consumer should stop cleanly.
		DeviceLost,	 // VK_ERROR_DEVICE_LOST
		RuntimeLost,	 // XR_ERROR_SESSION_LOST / XR_ERROR_INSTANCE_LOST /
					 // XR_SESSION_STATE_LOSS_PENDING
		OutOfMemory,	 // VK_ERROR_OUT_OF_{HOST,DEVICE}_MEMORY
		RuntimeError	 // anything else / GLFW errors
	};

	/**
	 * @brief Tells whether an error represents success.
	 */
	[[nodiscard]] constexpr bool isOk(Error error)
	{
		return error == Error::Ok;
	}

	/**
	 * @brief Tells whether an error is unrecoverable and should stop the
	 * application.
	 */
	[[nodiscard]] constexpr bool isFatal(Error error)
	{
		switch (error) {
			case Error::Ok:
			case Error::Suboptimal:
			case Error::SwapchainOutOfDate:
			case Error::NotReady:
				return false;
			case Error::DeviceLost:
			case Error::RuntimeLost:
			case Error::OutOfMemory:
			case Error::RuntimeError:
				return true;
		}
		return true;
	}

	/**
	 * @brief Tells whether an error is recoverable (non-fatal, non-success).
	 */
	[[nodiscard]] constexpr bool isRecoverable(Error error)
	{
		return !isOk(error) && !isFatal(error);
	}

	/**
	 * @brief Small result wrapper used by helpers that must return both a
	 * status and a value (e.g. createImageView).
	 *
	 * @tparam T The value type (must be default-constructible, which Vulkan
	 * handles and std::vector/std::shared_ptr satisfy).
	 */
	template <typename T>
	struct Result {
		Error code = Error::Ok;
		T value {};
	};

	/**
	 * @brief Maps a Vulkan result code into the normalized taxonomy.
	 *
	 * @param result The Vulkan result to map.
	 * @return The normalized error.
	 */
	[[nodiscard]] Error mapVkResult(VkResult result);
}	 // namespace evan
