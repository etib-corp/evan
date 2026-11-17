/*
 Copyright (c) 2026 ETIB Corporation

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include <evan/RenderSettings.hpp>
#include <evan/Renderer.hpp>

#include <utility/graphic/view.hpp>

#include <glm/glm.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <limits>
#include <vector>

namespace xider::tests
{
	TEST(TestRenderSettings, DefaultsDisableMultisampling)
	{
		const evan::RenderSettings settings;

		EXPECT_EQ(settings.msaaSamples, VK_SAMPLE_COUNT_1_BIT);
		EXPECT_EQ(settings.opaqueSort, evan::OpaqueSortMode::FrontToBack);
	}

	TEST(TestRenderSettings, DefaultsToStaticBackFaceCulling)
	{
		const evan::RenderSettings settings;

		EXPECT_EQ(settings.cullMode, evan::CullMode::Back);
		// Culling is static pipeline state, so it must not be a dynamic state.
		EXPECT_EQ(evan::toVkCullMode(evan::CullMode::Back),
				  VK_CULL_MODE_BACK_BIT);
		EXPECT_EQ(evan::toVkCullMode(evan::CullMode::Front),
				  VK_CULL_MODE_FRONT_BIT);
		EXPECT_EQ(evan::toVkCullMode(evan::CullMode::None), VK_CULL_MODE_NONE);
	}

	TEST(TestRenderSettings, SampleCubeOutwardWindingIsFrontFacing)
	{
		// The sample cube, counter-clockwise as seen from outside (the same
		// geometry as examples/GLFW-*-poc).
		const std::array<glm::vec3, 8> corners { {
			{ -0.5f, -0.5f, -0.5f },
			{ 0.5f, -0.5f, -0.5f },
			{ 0.5f, 0.5f, -0.5f },
			{ -0.5f, 0.5f, -0.5f },
			{ -0.5f, -0.5f, 0.5f },
			{ 0.5f, -0.5f, 0.5f },
			{ 0.5f, 0.5f, 0.5f },
			{ -0.5f, 0.5f, 0.5f },
		} };
		const std::array<uint32_t, 36> indices {
			0, 3, 2, 0, 2, 1,	 // back   (-Z)
			4, 5, 6, 4, 6, 7,	 // front  (+Z)
			0, 1, 5, 0, 5, 4,	 // bottom (-Y)
			3, 7, 6, 3, 6, 2,	 // top    (+Y)
			0, 4, 7, 0, 7, 3,	 // left   (-X)
			1, 2, 6, 1, 6, 5,	 // right  (+X)
		};

		utility::graphic::ViewF view;
		view.setPerspective(glm::radians(60.0f), 4.0f / 3.0f);
		view.setClippingPlanes(0.1f, 100.0f);

		// The default pose looks down -Z from the origin. Offset the cube so a
		// generic viewpoint sees three of its faces, none exactly edge-on.
		const glm::vec3 cubeOffset(2.0f, 1.0f, -6.0f);
		const glm::mat4 viewMatrix = view.toViewMatrix();
		const glm::mat4 projection = view.getProjectionMatrix();

		const auto project = [&](const glm::vec3 &position) {
			const glm::vec4 clip =
				projection * (viewMatrix * glm::vec4(position, 1.0f));

			return glm::vec3(clip) / clip.w;
		};

		std::size_t visibleTriangles = 0;
		std::size_t hiddenTriangles	 = 0;
		for (std::size_t i = 0; i < indices.size(); i += 3) {
			const glm::vec3 a = corners[indices[i]] + cubeOffset;
			const glm::vec3 b = corners[indices[i + 1]] + cubeOffset;
			const glm::vec3 c = corners[indices[i + 2]] + cubeOffset;

			// The counter-clockwise outside winding makes this normal point
			// away from the cube, so a positive dot with the camera direction
			// marks a face the viewer can see.
			const glm::vec3 normal = glm::cross(b - a, c - a);
			const glm::vec3 center = (a + b + c) / 3.0f;
			const bool facesCamera = glm::dot(normal, -center) > 0.0f;

			// Vulkan defines the polygon area in framebuffer coordinates as
			// a = -1/2 * sum(x_i * y_{i+1} - x_{i+1} * y_i). Converting NDC
			// to framebuffer pixels is a positive-affine scale, so this
			// NDC shoelace sum keeps the sign of -a. With kFrontFace set to
			// COUNTER_CLOCKWISE, front-facing triangles have a positive area,
			// i.e. a negative shoelace sum.
			const glm::vec3 na = project(a);
			const glm::vec3 nb = project(b);
			const glm::vec3 nc = project(c);
			const float shoelace =
				(nb.x - na.x) * (nc.y - na.y) - (nc.x - na.x) * (nb.y - na.y);

			if (facesCamera) {
				++visibleTriangles;
				EXPECT_LT(shoelace, 0.0f)
					<< "A visible outward triangle must be front-facing with "
					   "VK_FRONT_FACE_COUNTER_CLOCKWISE";
			} else {
				++hiddenTriangles;
				EXPECT_GT(shoelace, 0.0f)
					<< "A hidden outward triangle must be back-facing with "
					   "VK_FRONT_FACE_COUNTER_CLOCKWISE";
			}
		}

		// A generic viewpoint sees exactly three faces of a closed cube.
		EXPECT_EQ(visibleTriangles, 6u);
		EXPECT_EQ(hiddenTriangles, 6u);
		EXPECT_EQ(evan::kFrontFace, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	}

	TEST(TestRenderSettings, DesktopRendersDirectlyWithoutMultisampling)
	{
		EXPECT_EQ(evan::desktopColorAttachmentMode(VK_SAMPLE_COUNT_1_BIT),
				  evan::ColorAttachmentMode::DirectToSwapchain);
	}

	TEST(TestRenderSettings, DesktopResolvesWhenMultisampling)
	{
		EXPECT_EQ(evan::desktopColorAttachmentMode(VK_SAMPLE_COUNT_2_BIT),
				  evan::ColorAttachmentMode::ResolveToSwapchain);
		EXPECT_EQ(evan::desktopColorAttachmentMode(VK_SAMPLE_COUNT_4_BIT),
				  evan::ColorAttachmentMode::ResolveToSwapchain);
	}

	TEST(TestRenderSettings, DistancesWithoutDepthSortAfterEveryBucket)
	{
		constexpr uint32_t bucketCount = 8;

		// Meshes without usable bounds use the float maximum as a sentinel, and
		// a NaN can come out of a degenerate transform. Neither may be cast to
		// a bucket index.
		EXPECT_EQ(evan::opaqueDepthBucket(std::numeric_limits<float>::max(),
										  1.0f, 9.0f, bucketCount),
				  bucketCount);
		EXPECT_EQ(
			evan::opaqueDepthBucket(std::numeric_limits<float>::quiet_NaN(),
									1.0f, 9.0f, bucketCount),
			bucketCount);
	}

	TEST(TestRenderSettings, EmptyDepthRangeUsesTheTrailingBucket)
	{
		// Every draw sits at the same distance: there is no ordering to apply.
		EXPECT_EQ(evan::opaqueDepthBucket(5.0f, 5.0f, 0.0f, 8u), 8u);
	}

	TEST(TestRenderSettings, DepthBucketsAreOrderedAndClamped)
	{
		constexpr uint32_t bucketCount = 8;
		constexpr float nearest		   = 10.0f;
		constexpr float span		   = 80.0f;

		EXPECT_EQ(evan::opaqueDepthBucket(nearest, nearest, span, bucketCount),
				  0u);

		// Past the visible range and infinity saturate on the last bucket
		// instead of overflowing the float to integer conversion.
		EXPECT_EQ(evan::opaqueDepthBucket(nearest + span * 2.0f, nearest, span,
										  bucketCount),
				  bucketCount - 1);
		EXPECT_EQ(
			evan::opaqueDepthBucket(std::numeric_limits<float>::infinity(),
									nearest, span, bucketCount),
			bucketCount - 1);

		uint32_t previous =
			evan::opaqueDepthBucket(nearest, nearest, span, bucketCount);
		for (float offset = 0.0f; offset <= span; offset += span / 16.0f) {
			const uint32_t bucket = evan::opaqueDepthBucket(
				nearest + offset, nearest, span, bucketCount);
			EXPECT_GE(bucket, previous);
			EXPECT_LT(bucket, bucketCount);
			previous = bucket;
		}
	}

	TEST(TestRenderSettings, PipelineVariantsOrderByShaderBlendAndCullMode)
	{
		const evan::Renderer::PipelineVariant firstOpaque {
			1, evan::BlendMode::Opaque, evan::CullMode::Back
		};
		const evan::Renderer::PipelineVariant firstAlpha {
			1, evan::BlendMode::Alpha, evan::CullMode::Back
		};
		const evan::Renderer::PipelineVariant secondOpaque {
			2, evan::BlendMode::Opaque, evan::CullMode::Back
		};
		const evan::Renderer::PipelineVariant firstOpaqueNone {
			1, evan::BlendMode::Opaque, evan::CullMode::None
		};

		EXPECT_TRUE(firstOpaqueNone < firstOpaque);
		EXPECT_TRUE(firstOpaque < firstAlpha);
		EXPECT_TRUE(firstAlpha < secondOpaque);
		EXPECT_FALSE(firstOpaque < firstOpaque);
		EXPECT_FALSE(secondOpaque < firstAlpha);
	}

	TEST(TestRenderSettings, OpaqueDrawsSortBeforeAlphaDraws)
	{
		evan::DrawSortKey opaque;
		opaque.depthBucket = 9;
		evan::DrawSortKey alpha;
		alpha.blendMode = evan::BlendMode::Alpha;
		alpha.depth		= 1.0f;

		// The partition is decided before any depth: an opaque draw is never
		// recorded after a blended one, whatever their distances are.
		EXPECT_TRUE(evan::drawSortKeyLess(opaque, alpha));
		EXPECT_FALSE(evan::drawSortKeyLess(alpha, opaque));
	}

	TEST(TestRenderSettings, OpaqueDrawsSortFrontToBackInsideStateGroups)
	{
		evan::DrawSortKey nearDraw;
		nearDraw.depthBucket = 1;
		evan::DrawSortKey farDraw;
		farDraw.depthBucket = 4;

		EXPECT_TRUE(evan::drawSortKeyLess(nearDraw, farDraw));
		EXPECT_FALSE(evan::drawSortKeyLess(farDraw, nearDraw));

		// Same bucket: pipeline, material and geometry decide, so identical
		// geometry stays contiguous and keeps merging into a batch.
		evan::DrawSortKey lowPipeline;
		lowPipeline.depthBucket = 4;
		lowPipeline.pipelineKey = 3;
		evan::DrawSortKey highPipeline;
		highPipeline.depthBucket = 4;
		highPipeline.pipelineKey = 7;

		EXPECT_TRUE(evan::drawSortKeyLess(lowPipeline, highPipeline));
		EXPECT_FALSE(evan::drawSortKeyLess(highPipeline, lowPipeline));
	}

	TEST(TestRenderSettings, AlphaDrawsSortBackToFrontByDepthFirst)
	{
		evan::DrawSortKey nearDraw;
		nearDraw.blendMode	 = evan::BlendMode::Alpha;
		nearDraw.depth		 = 2.0f;
		nearDraw.pipelineKey = 1;
		evan::DrawSortKey farDraw;
		farDraw.blendMode	= evan::BlendMode::Alpha;
		farDraw.depth		= 40.0f;
		farDraw.pipelineKey = 9;

		// Distance wins over state: grouping by pipeline first would composite
		// the two materials in the wrong order.
		EXPECT_TRUE(evan::drawSortKeyLess(farDraw, nearDraw));
		EXPECT_FALSE(evan::drawSortKeyLess(nearDraw, farDraw));
	}

	TEST(TestRenderSettings, DrawSortKeyIsATotalOrder)
	{
		evan::DrawSortKey first;
		first.sequence = 1;
		evan::DrawSortKey second;
		second.sequence = 2;

		// Every field but the sequence is equal. The sequence still orders
		// them, which makes the order total instead of leaving equal draws to
		// std::sort's discretion.
		EXPECT_TRUE(evan::drawSortKeyLess(first, second));
		EXPECT_FALSE(evan::drawSortKeyLess(second, first));
		EXPECT_FALSE(evan::drawSortKeyLess(first, first));
	}

	TEST(TestRenderSettings, SortingIsReproducible)
	{
		std::vector<evan::DrawSortKey> ascending(4);
		for (std::size_t i = 0; i < ascending.size(); ++i) {
			ascending[i].sequence = static_cast<uint32_t>(i);
		}

		// Same draws in the opposite order: a total order has to produce the
		// same sequence, otherwise recordings are not reproducible run to run.
		std::vector<evan::DrawSortKey> descending = ascending;
		std::reverse(descending.begin(), descending.end());

		const auto less = [](const evan::DrawSortKey &lhs,
							 const evan::DrawSortKey &rhs) {
			return evan::drawSortKeyLess(lhs, rhs);
		};
		std::sort(ascending.begin(), ascending.end(), less);
		std::sort(descending.begin(), descending.end(), less);

		for (std::size_t i = 0; i < ascending.size(); ++i) {
			EXPECT_EQ(ascending[i].sequence, i);
			EXPECT_EQ(descending[i].sequence, i);
		}
	}
}	 // namespace xider::tests
