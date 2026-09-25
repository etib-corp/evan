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
#include <evan/Renderer.hpp>

#include <cstddef>

#include <gtest/gtest.h>

namespace xider::tests
{
	/**
	 * @brief The per-frame slot count must follow the real view/swapchain
	 * count, with MAX_SWAPCHAINS kept only as an upper bound.
	 */
	TEST(FrameSlotCountTest, DesktopSingleViewSingleSwapchain)
	{
		EXPECT_EQ(::frameSlotCount(1, 1), 1u);
	}

	TEST(FrameSlotCountTest, XrStereoTwoViewsTwoSwapchains)
	{
		EXPECT_EQ(::frameSlotCount(2, 2), 2u);
	}

	TEST(FrameSlotCountTest, MultiViewSharingSingleSwapchain)
	{
		// More views than swapchains: the view-indexed resources still need
		// one slot per view, so the larger count wins.
		EXPECT_EQ(::frameSlotCount(2, 1), 2u);
	}

	TEST(FrameSlotCountTest, ClampedToMaxSwapchains)
	{
		const std::size_t maxSlots = static_cast<std::size_t>(MAX_SWAPCHAINS);

		EXPECT_EQ(::frameSlotCount(maxSlots + 1, 1), maxSlots);
		EXPECT_EQ(::frameSlotCount(100, 100), maxSlots);
	}

	TEST(FrameSlotCountTest, NeverAllocatesZeroSlots)
	{
		EXPECT_EQ(::frameSlotCount(0, 0), 1u);
	}

	/**
	 * @brief Regression guard for the P0-03 invariant: exactly one in-flight
	 * fence per frame, attached to the last submitted view.
	 *
	 * If this ever returns true for another view (or for no view at all), the
	 * CPU would wait for each eye in turn again.
	 */
	TEST(RendererSubmissionPlanTest, SingleFenceOnLastView)
	{
		for (std::size_t viewCount = 1;
			 viewCount <= static_cast<std::size_t>(MAX_SWAPCHAINS);
			 ++viewCount) {
			std::size_t fencedCount = 0;
			for (std::size_t viewIndex = 0; viewIndex < viewCount;
				 ++viewIndex) {
				if (evan::Renderer::viewOwnsInFlightFence(viewIndex,
														  viewCount)) {
					++fencedCount;
					EXPECT_EQ(viewIndex, viewCount - 1)
						<< "fence attached to a non-final view "
						<< "(viewCount=" << viewCount << ")";
				}
			}
			EXPECT_EQ(fencedCount, 1u)
				<< "expected exactly one fenced view (viewCount=" << viewCount
				<< ")";
		}
	}

	TEST(RendererSubmissionPlanTest, NoFenceWithoutViews)
	{
		EXPECT_FALSE(evan::Renderer::viewOwnsInFlightFence(0, 0));
	}
}	 // namespace xider::tests
