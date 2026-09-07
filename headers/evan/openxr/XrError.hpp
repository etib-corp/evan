/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrError
*/

#pragma once

#include "evan/Error.hpp"

#include <openxr/openxr.h>

namespace evan
{
	/**
	 * @brief Maps an OpenXR result code into the normalized taxonomy.
	 *
	 * @param result The OpenXR result to map.
	 * @return The normalized error.
	 */
	[[nodiscard]] Error mapXrResult(XrResult result);

	/**
	 * @brief Maps an OpenXR session state into the normalized taxonomy.
	 *
	 * @param state The session state to map.
	 * @return The normalized error. XR_SESSION_STATE_LOSS_PENDING maps to
	 * RuntimeLost; all other states map to Ok.
	 */
	[[nodiscard]] Error mapSessionState(XrSessionState state);
}	 // namespace evan
