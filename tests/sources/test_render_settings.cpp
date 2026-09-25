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

#include <gtest/gtest.h>

#include <limits>

namespace xider::tests
{
	TEST(TestRenderSettings, DefaultsDisableMultisampling)
	{
		const evan::RenderSettings settings;

		EXPECT_EQ(settings.msaaSamples, VK_SAMPLE_COUNT_1_BIT);
		EXPECT_EQ(settings.opaqueSort, evan::OpaqueSortMode::FrontToBack);
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

	TEST(TestRenderSettings, PipelineVariantsOrderByShaderThenBlendMode)
	{
		const evan::Renderer::PipelineVariant firstOpaque {
			1, evan::BlendMode::Opaque
		};
		const evan::Renderer::PipelineVariant firstAlpha {
			1, evan::BlendMode::Alpha
		};
		const evan::Renderer::PipelineVariant secondOpaque {
			2, evan::BlendMode::Opaque
		};

		EXPECT_TRUE(firstOpaque < firstAlpha);
		EXPECT_TRUE(firstAlpha < secondOpaque);
		EXPECT_FALSE(firstOpaque < firstOpaque);
		EXPECT_FALSE(secondOpaque < firstAlpha);
	}
}	 // namespace xider::tests
