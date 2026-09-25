/*
** ETIB PROJECT, 2026
** evan
** File description:
** RenderSettings
*/

#pragma once

#include "evan/EvanPlatform.hpp"

#include <cstdint>
#include <limits>

namespace evan
{
	/**
	 * @brief How a draw's color output is combined with the framebuffer.
	 *
	 * Part of the graphics pipeline key: one pipeline is created per
	 * (shader, blend mode) pair so that blending is only enabled for the
	 * geometry that needs it.
	 */
	enum class BlendMode {
		Opaque,	   ///< Blending disabled: the fragment output replaces the
				   ///< framebuffer content and depth writes are enabled.
		Alpha	   ///< Source-alpha over destination, depth writes disabled.
				   ///< Those draws have to be issued back-to-front.
	};

	/**
	 * @brief Ordering strategy used for the opaque draws of a frame.
	 */
	enum class OpaqueSortMode {
		StateFirst,	   ///< Group draws sharing pipeline, material and
					   ///< geometry first, to maximize batching.
		FrontToBack	   ///< Bucket draws front-to-back first, then group by
					   ///< state inside a bucket, to reduce overdraw.
	};

	/**
	 * @brief How a render pass obtains its single-sampled color attachment.
	 *
	 * Derived from the effective sample count and from who owns the swapchain
	 * images. Shared by the swapchain contexts, the swapchain images and the
	 * render pass; it is plumbing, not a per-application setting.
	 */
	enum class ColorAttachmentMode {
		DirectToSwapchain,	   ///< The swapchain image is the color attachment
							   ///< and the render pass ends in
							   ///< VK_IMAGE_LAYOUT_PRESENT_SRC_KHR. Used for a
							   ///< single-sampled desktop swapchain.
		ResolveToSwapchain,	   ///< A multisampled color attachment is resolved
							   ///< into the swapchain image, which ends in
							   ///< VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
		SwapchainImageAttachment	///< The swapchain image is the color
									///< attachment and is left in
									///< VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.
									///< Used by OpenXR, where the runtime owns
									///< the images and may hand out
									///< multisampled ones.
	};

	/**
	 * @brief Selects the color attachment layout of a desktop-style
	 * presentation swapchain, whose images are single-sampled.
	 *
	 * With a single sample there is nothing to resolve, so the render pass
	 * draws straight into the swapchain image: no multisampled color image is
	 * allocated and no resolve bandwidth is paid.
	 *
	 * @param msaaSamples The effective sample count of the render pass.
	 * @return The matching color attachment mode.
	 */
	[[nodiscard]] inline ColorAttachmentMode
		desktopColorAttachmentMode(VkSampleCountFlagBits msaaSamples) noexcept
	{
		return msaaSamples == VK_SAMPLE_COUNT_1_BIT
			? ColorAttachmentMode::DirectToSwapchain
			: ColorAttachmentMode::ResolveToSwapchain;
	}

	/**
	 * @brief Number of depth buckets used to sort opaque draws front-to-back.
	 *
	 * Draws landing in the same bucket keep their state ordering, so meshes
	 * sharing pipeline, material and geometry stay adjacent and can still be
	 * merged into instanced or indirect draws. A single bucket means no depth
	 * ordering at all, one bucket per draw means no state batching.
	 */
	constexpr uint32_t kOpaqueDepthBucketCount = 32;

	/**
	 * @brief Distance given to draws whose distance from the camera is
	 * unknown.
	 *
	 * A mesh without usable bounds has no center to measure from, so the
	 * renderer keeps it out of the front-to-back order by giving it the
	 * largest representable distance, which no measured distance reaches. It
	 * is the value the depth bucket function recognizes as the sentinel.
	 */
	constexpr float kUnknownDepth = std::numeric_limits<float>::max();

	/**
	 * @brief Maps a camera distance onto its depth bucket.
	 *
	 * Distances are normalized against the visible range so that the bucket
	 * size adapts to the scene scale instead of assuming world units. A
	 * distance that is unknown or not comparable (a NaN, or kUnknownDepth,
	 * the sentinel used for meshes without bounds) maps to the extra bucket
	 * past the last one, which keeps those draws at the end of the
	 * front-to-back order.
	 *
	 * @param depth Distance from the camera to the draw.
	 * @param nearest Closest distance among the draws being ordered.
	 * @param span Extent of the visible distance range.
	 * @param bucketCount Number of buckets, excluding the trailing one.
	 * @return The bucket index, in [0, bucketCount].
	 */
	[[nodiscard]] inline uint32_t
		opaqueDepthBucket(float depth, float nearest, float span,
						  uint32_t bucketCount) noexcept
	{
		// A draw whose distance is unknown (kUnknownDepth) or cannot be
		// compared against the others (a NaN, or every draw when the visible
		// range is empty) has no place inside the front-to-back order: it
		// goes to the trailing bucket, past the last real one. The sentinel
		// is checked explicitly because it is a finite value, so it would
		// otherwise be clamped onto the last real bucket like any distance
		// past the visible range.
		if (!(span > 0.0f) || !(depth >= nearest) || depth == kUnknownDepth) {
			return bucketCount;
		}

		const float normalized = (depth - nearest) / span;
		// Infinity and distances past the visible range saturate on the last
		// real bucket before the conversion to avoid an out-of-range cast.
		if (!(normalized < 1.0f)) {
			return bucketCount - 1;
		}

		return static_cast<uint32_t>(normalized
									 * static_cast<float>(bucketCount));
	}

	/**
	 * @brief Application-facing rendering configuration.
	 *
	 * Passed to the Engine, which applies it before the swapchain and its
	 * render pass are created: the sample count is baked into the render pass
	 * and the framebuffers, so it is a construction-time setting.
	 */
	struct RenderSettings {
		/**
		 * @brief Multisample anti-aliasing sample count.
		 *
		 * Defaults to a single sample. MSAA multiplies color and depth
		 * bandwidth by the sample count and forces a resolve, which every
		 * scene pays whether or not it benefits from the extra edge quality.
		 * Raise it to VK_SAMPLE_COUNT_2_BIT or VK_SAMPLE_COUNT_4_BIT when it
		 * is worth the bandwidth. The EVAN_MSAA environment variable
		 * overrides this value, which makes A/B measurements possible without
		 * a rebuild.
		 */
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		/**
		 * @brief Ordering strategy for the opaque draws of a frame.
		 */
		OpaqueSortMode opaqueSort = OpaqueSortMode::FrontToBack;
	};
}	 // namespace evan
