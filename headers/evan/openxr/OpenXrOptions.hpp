/*
** ETIB PROJECT, 2026
** evan
** File description:
** OpenXrOptions
*/

#pragma once

#include <openxr/openxr.h>

#include <cstdint>
#include <string>
#include <vector>

namespace evan
{
	/**
	 * @brief Features that can be enabled in the OpenXR backend.
	 *
	 * These features are not guaranteed to be supported by the runtime, but
	 * they can be requested and the runtime will report whether they are
	 * available.
	 */
	enum class ArFeature : std::uint32_t {
		None		 = 0,
		Passthrough	 = 1u << 0,
		HandTracking = 1u << 1,
		HandMesh	 = 1u << 2,
		Scene		 = 1u << 3,
		Anchors		 = 1u << 4,
		Depth		 = 1u << 5,
	};

	/**
	 * @brief Combines two AR feature sets.
	 */
	constexpr ArFeature operator|(ArFeature lhs, ArFeature rhs) noexcept
	{
		return static_cast<ArFeature>(static_cast<std::uint32_t>(lhs)
									  | static_cast<std::uint32_t>(rhs));
	}

	/**
	 * @brief Intersects two AR feature sets.
	 */
	constexpr ArFeature operator&(ArFeature lhs, ArFeature rhs) noexcept
	{
		return static_cast<ArFeature>(static_cast<std::uint32_t>(lhs)
									  & static_cast<std::uint32_t>(rhs));
	}

	/**
	 * @brief Whether @p feature is part of @p features.
	 */
	constexpr bool hasFeature(ArFeature features, ArFeature feature) noexcept
	{
		return (features & feature) != ArFeature::None;
	}

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

		/**
		 * @brief Whether AR instance extensions may be requested.
		 *
		 * Disabled by default so VR-only apps keep today's exact behavior.
		 */
		bool enableArExtensions = false;

		/**
		 * @brief AR features whose extensions should be requested.
		 *
		 * Only used when enableArExtensions is true. Unsupported extensions are
		 * filtered out instead of failing instance creation.
		 */
		ArFeature minimumArFeatures =
			ArFeature::Passthrough | ArFeature::HandTracking;
	};
}	 // namespace evan
