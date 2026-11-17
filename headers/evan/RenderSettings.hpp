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
	 * @brief Which triangle faces the rasterizer discards.
	 *
	 * Face culling is static pipeline state: it is baked into every pipeline
	 * the renderer creates and cannot be changed per draw without a new
	 * pipeline. `VK_DYNAMIC_STATE_CULL_MODE` is deliberately not used, because
	 * it belongs to `VK_EXT_extended_dynamic_state` (Vulkan 1.3), which the
	 * device feature chain does not enable.
	 *
	 * @note This is not the frustum/distance culling controlled by
	 * Renderer::setCullingEnabled(); that culling decides whether a mesh is
	 * drawn at all, while this mode decides which faces of a drawn triangle
	 * reach the framebuffer.
	 */
	enum class CullMode {
		None,	 ///< Rasterize both faces: double-sided content.
		Back,	 ///< Discard back faces; the default, and what is right for the
				 ///< closed meshes of the samples.
		Front	 ///< Discard front faces (inside-out content and debugging).
	};

	/**
	 * @brief Front-face winding baked into every pipeline.
	 *
	 * View::getProjectionMatrix() flips Y for Vulkan NDC by default, which
	 * keeps a Y-up world upright on the Y-down Vulkan framebuffer: a positive
	 * world Y maps to a negative NDC Y, i.e. the top of the screen. The flip
	 * cancels the framebuffer's own Y-down convention, so the
	 * counter-clockwise, outside-facing winding of the sample meshes stays
	 * counter-clockwise in framebuffer coordinates and is front-facing with
	 * VK_FRONT_FACE_COUNTER_CLOCKWISE.
	 *
	 * Vulkan defines the polygon area in framebuffer coordinates as
	 * `a = -1/2 * sum(x_i * y_{i+1} - x_{i+1} * y_i)`; a positive area is
	 * front-facing for VK_FRONT_FACE_COUNTER_CLOCKWISE. This matches the
	 * "counter-clockwise as seen from outside" geometry convention without
	 * changing the viewport.
	 *
	 * @note Changing the projection to stop flipping Y (or using a negative
	 * viewport height instead) changes this pairing; see
	 * POC_RESULTS_P3.md §7.
	 */
	inline constexpr VkFrontFace kFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	/**
	 * @brief Maps a CullMode onto its static rasterization flag.
	 *
	 * @param mode The application-facing culling policy.
	 * @return The matching VkCullModeFlags value.
	 */
	[[nodiscard]] inline constexpr VkCullModeFlags
		toVkCullMode(CullMode mode) noexcept
	{
		switch (mode) {
			case CullMode::None:
				return VK_CULL_MODE_NONE;
			case CullMode::Front:
				return VK_CULL_MODE_FRONT_BIT;
			case CullMode::Back:
				break;
		}
		return VK_CULL_MODE_BACK_BIT;
	}

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
	 * @brief Key that decides in which order a frame's draws are recorded.
	 *
	 * Resolved once per draw before sorting, so that ordering is a plain key
	 * comparison instead of work repeated inside the comparator. It carries, in
	 * decreasing priority:
	 *
	 * - the blend mode, which partitions opaque draws before blended ones;
	 * - for opaque draws, the depth bucket, ascending (front-to-back), so that
	 *   early depth testing can reject occluded fragments; for blended draws,
	 *   the exact distance, descending (back-to-front), so that compositing
	 *   stays correct;
	 * - the state identifying a batch: pipeline, material and geometry;
	 * - a unique sequence number, which makes the order total and therefore
	 *   reproducible whatever std::sort does with equal elements.
	 */
	struct DrawSortKey {
		BlendMode blendMode	  = BlendMode::Opaque;
		uint32_t depthBucket  = 0;
		float depth			  = 0.0f;
		uint32_t pipelineKey  = 0;
		uint32_t materialID	  = 0;
		VkBuffer vertexBuffer = VK_NULL_HANDLE;
		VkBuffer indexBuffer  = VK_NULL_HANDLE;
		uint32_t sequence	  = 0;
	};

	/**
	 * @brief Strict total order over DrawSortKey.
	 *
	 * Opaque draws come first and are ordered front-to-back by depth bucket.
	 * Blended draws follow and are ordered back-to-front by exact distance,
	 * because their state must not be grouped ahead of their depth. Inside a
	 * bucket (opaque) or a distance (alpha), state groups stay contiguous so
	 * that the run merging used by instanced and indirect drawing still
	 * applies. The unique sequence number breaks the remaining ties.
	 *
	 * @param lhs Left key.
	 * @param rhs Right key.
	 * @return True when @p lhs must be recorded before @p rhs.
	 */
	[[nodiscard]] inline bool drawSortKeyLess(const DrawSortKey &lhs,
											  const DrawSortKey &rhs) noexcept
	{
		if (lhs.blendMode != rhs.blendMode) {
			return static_cast<int>(lhs.blendMode)
				< static_cast<int>(rhs.blendMode);
		}

		if (lhs.blendMode == BlendMode::Alpha) {
			// Blended geometry is composited back-to-front. Depth decides
			// before state: grouping materials by pipeline first would blend
			// them in the wrong order.
			if (lhs.depth != rhs.depth) {
				return lhs.depth > rhs.depth;
			}
		} else if (lhs.depthBucket != rhs.depthBucket) {
			// Opaque geometry goes front-to-back. The bucket, not the exact
			// distance, so that draws of the same bucket keep their state
			// ordering and still merge into batching.
			return lhs.depthBucket < rhs.depthBucket;
		}

		if (lhs.pipelineKey != rhs.pipelineKey) {
			return lhs.pipelineKey < rhs.pipelineKey;
		}
		if (lhs.materialID != rhs.materialID) {
			return lhs.materialID < rhs.materialID;
		}
		if (lhs.vertexBuffer != rhs.vertexBuffer) {
			return lhs.vertexBuffer < rhs.vertexBuffer;
		}
		if (lhs.indexBuffer != rhs.indexBuffer) {
			return lhs.indexBuffer < rhs.indexBuffer;
		}
		return lhs.sequence < rhs.sequence;
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

		/**
		 * @brief Face-culling policy baked into the graphics pipelines.
		 *
		 * Defaults to back-face culling with the front face settled by
		 * kFrontFace. Pass CullMode::None to rasterize both faces of every
		 * mesh, or override the policy for a single material through
		 * Renderer::setMaterialCullMode() (double-sided content, such as a
		 * camera-facing quad).
		 *
		 * The pipelines are created with this mode when the Renderer is
		 * constructed, so it is a construction-time setting. The
		 * EVAN_CULL_MODE environment variable (`none`, `back`, `front`)
		 * overrides it, which makes A/B measurements possible without a
		 * rebuild.
		 */
		CullMode cullMode = CullMode::Back;
	};
}	 // namespace evan
