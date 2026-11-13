/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrPassthrough
*/

#pragma once

#include <openxr/openxr.h>

#include "evan/Error.hpp"
#include "evan/openxr/XrExtensions.hpp"

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

namespace evan
{
	/**
	 * @brief Owns the OpenXR passthrough feature and its passthrough layer.
	 *
	 * The subsystem is created from a running XrSession and starts/stops the
	 * passthrough objects as a unit. It does not submit the composition layer;
	 * AR-102 consumes getLayer() for that.
	 */
	class XrPassthrough:
		protected utility::logging::Loggable<XrPassthrough,
											 utility::logging::DefaultLogger>
	{
		public:
		/**
		 * @brief Creates the subsystem for @p session.
		 *
		 * @param session The OpenXR session owning the passthrough objects.
		 * @param functions The loaded XR_FB_passthrough function pointers.
		 */
		XrPassthrough(XrSession session,
					  const XrInstanceFunctions::Passthrough &functions);

		XrPassthrough(const XrPassthrough &)			= delete;
		XrPassthrough &operator=(const XrPassthrough &) = delete;

		~XrPassthrough() override;

		/**
		 * @brief Whether the session and every required function are usable.
		 */
		[[nodiscard]] bool isSupported() const noexcept;

		/**
		 * @brief Creates the feature/layer if needed and starts passthrough.
		 *
		 * Idempotent: calling start() while running returns Error::Ok.
		 */
		Error start();

		/**
		 * @brief Pauses and destroys the feature/layer.
		 *
		 * Idempotent: calling stop() while stopped returns Error::Ok.
		 */
		Error stop();

		[[nodiscard]] bool isRunning() const noexcept;

		/**
		 * @brief The layer handle to submit from AR-102.
		 *
		 * Valid only while isRunning() is true.
		 */
		[[nodiscard]] XrPassthroughLayerFB getLayer() const noexcept;

		private:
		XrSession _session = XR_NULL_SESSION;
		XrInstanceFunctions::Passthrough _functions;
		XrPassthroughFB _passthrough = XR_NULL_HANDLE;
		XrPassthroughLayerFB _layer	 = XR_NULL_HANDLE;
		bool _running				 = false;
	};
}	 // namespace evan
