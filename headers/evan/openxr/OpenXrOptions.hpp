/*
** ETIB PROJECT, 2026
** evan
** File description:
** OpenXrOptions
*/

#pragma once

#include <openxr/openxr.h>

#include <string>
#include <vector>

namespace evan
{
	/**
	 * @brief User-configurable options for the OpenXR backend.
	 *
	 * Instances are owned by the OpenXR platform (see IXrPlatform) and copied
	 * into the device backend at construction time so that the swapchain,
	 * action and interaction profile subsystems can read them without reaching
	 * back into the platform.
	 */
	struct OpenXrOptions {
		/**
		 * @brief Interaction profiles to try, in preference order.
		 *
		 * The first profile reported as supported by the active runtime is used
		 * to suggest action bindings. When empty, a built-in default list is
		 * used (see InteractionProfile::getDefaultProfileCandidates()).
		 */
		std::vector<std::string> interactionProfileCandidates;

		/**
		 * @brief Swapchain usage flags used when creating OpenXR swapchains.
		 *
		 * The OpenXR core specification does not expose runtime-recommended
		 * swapchain usage flags, so the engine uses this configurable default.
		 * The default is suitable for rendering and sampling the final image.
		 */
		XrSwapchainUsageFlags swapchainUsageFlags =
			XR_SWAPCHAIN_USAGE_SAMPLED_BIT
			| XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;

		/**
		 * @brief Whether menu/system button bindings are suggested.
		 *
		 * Disabled by default because some runtimes reserve these inputs (e.g.
		 * the system button). When enabled, bindings that the runtime rejects
		 * are dropped and the remaining bindings are re-suggested so the core
		 * actions keep working.
		 */
		bool enableMenuSystemBindings = false;
	};
}	 // namespace evan
