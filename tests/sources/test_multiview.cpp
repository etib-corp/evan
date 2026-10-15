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

#include <evan/EvanPlatform.hpp>
#include <evan/Frame.hpp>

#include <cstddef>
#include <cstdint>

#include <glm/glm.hpp>

#include <gtest/gtest.h>

namespace xider::tests
{
	/**
	 * @brief The render-pass view mask is only built for stereo-like view
	 * counts; everything else must keep the single-view path.
	 */
	TEST(MultiviewViewMaskTest, MonoAndEmptyFallBackToSingleView)
	{
		EXPECT_EQ(::multiviewViewMask(0), 0u);
		EXPECT_EQ(::multiviewViewMask(1), 0u);
	}

	TEST(MultiviewViewMaskTest, StereoEnablesBothViews)
	{
		EXPECT_EQ(::multiviewViewMask(2), 0b11u);
	}

	TEST(MultiviewViewMaskTest, LargerConfigurationsFallBackToSingleView)
	{
		EXPECT_EQ(::multiviewViewMask(3), 0u);
		EXPECT_EQ(::multiviewViewMask(100), 0u);
	}

	TEST(MultiviewViewMaskTest, BoundMatchesTheUniformLayout)
	{
		// The uniform block, the mask and the renderer cap must agree on the
		// number of views a single multiview pass can carry.
		EXPECT_EQ(MAX_MULTIVIEW_VIEWS, 2u);
		EXPECT_EQ(::multiviewViewMask(MAX_MULTIVIEW_VIEWS),
				  (1u << MAX_MULTIVIEW_VIEWS) - 1u);
	}

	/**
	 * @brief The CPU-side uniform block must match the std140 layout the
	 * example shaders are built against: model at offset 0, then one
	 * view/projection pair per view at a 128-byte stride.
	 *
	 * The example vertex shaders read ubo.views[gl_ViewIndex]; if this layout
	 * drifts, both the multiview pass and the per-slot fallback break.
	 */
	TEST(MultiviewUniformLayoutTest, MatchesShaderStd140Layout)
	{
		using evan::Frame;

		EXPECT_EQ(sizeof(Frame::ViewUniform), 128u);
		EXPECT_EQ(sizeof(Frame::UniformBufferObject), 320u);

		Frame::UniformBufferObject ubo {};
		const auto base	  = reinterpret_cast<const char *>(&ubo);
		const auto offset = [&base](const void *member) {
			return static_cast<std::size_t>(
				reinterpret_cast<const char *>(member) - base);
		};

		EXPECT_EQ(offset(&ubo.model), 0u);
		EXPECT_EQ(offset(&ubo.views[0]), 64u);
		EXPECT_EQ(offset(&ubo.views[1]), 192u);
		EXPECT_EQ(offset(&ubo.views[0].view), 64u);
		EXPECT_EQ(offset(&ubo.views[0].proj), 128u);
		EXPECT_EQ(offset(&ubo.views[1].view), 192u);
		EXPECT_EQ(offset(&ubo.views[1].proj), 256u);
	}

	/**
	 * @brief Fallback semantics: an eye written into its own slot must land in
	 * the entry the shader reads, which is index 0 because ViewIndex is zero
	 * outside multiview render passes.
	 */
	TEST(MultiviewUniformLayoutTest, FallbackSlotExposesMatrixAtEntryZero)
	{
		evan::Frame::UniformBufferObject slot {};
		slot.views[0].view = glm::mat4(2.0f);
		slot.views[1].proj = glm::mat4(3.0f);

		// The fallback writes one eye per slot; the shader reads entry 0.
		EXPECT_EQ(slot.views[0].view[0][0], 2.0f);
		// A multiview slot keeps both eyes addressable side by side, without
		// the second entry disturbing the first.
		EXPECT_EQ(slot.views[1].proj[0][0], 3.0f);
		EXPECT_EQ(slot.views[0].proj[0][0], 0.0f);
	}
}	 // namespace xider::tests
