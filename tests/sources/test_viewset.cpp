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

#include <evan/ASwapchainContext.hpp>
#include <evan/DeviceContext.hpp>
#include <evan/ViewSet.hpp>

#include <utility/graphic/view.hpp>

#include <gtest/gtest.h>

namespace xider::tests
{
	/**
	 * @brief Minimal mock swapchain context used to exercise the ViewSet
	 * integration without any real Vulkan or OpenXR objects.
	 */
	class MockSwapchainContext: public evan::ASwapchainContext
	{
		public:
		VkResult aquireImage(uint32_t index, VkDevice, VkSemaphore, VkFence,
							 uint32_t &imageIndex) override
		{
			imageIndex = index;
			return VK_SUCCESS;
		}

		void waitForImage(uint32_t) override
		{
		}

		void destroy(VkDevice) override
		{
		}

		void recreateSwapchain(const evan::DeviceContext &,
							   VkRenderPass) override
		{
		}

		evan::ViewSet &getViewSet() override
		{
			return _viewSet;
		}

		const evan::ViewSet &getViewSet() const override
		{
			return _viewSet;
		}

		evan::ViewSet _viewSet;
	};

	TEST(ViewSetTest, StartsEmpty)
	{
		evan::ViewSet views;

		EXPECT_TRUE(views.empty());
		EXPECT_EQ(views.size(), 0u);
	}

	TEST(ViewSetTest, SetViewGrowsSet)
	{
		evan::ViewSet views;
		utility::graphic::ViewF view;

		views.setView(2, view);

		EXPECT_EQ(views.size(), 3u);
		// Newly created entries default to swapchain index 0.
		EXPECT_EQ(views[0].swapchainIndex, 0u);
		EXPECT_EQ(views[1].swapchainIndex, 0u);
		EXPECT_EQ(views[2].swapchainIndex, 0u);
	}

	TEST(ViewSetTest, GetProjectionMatchesViewProjection)
	{
		evan::ViewSet views;
		utility::graphic::ViewF view;
		view.setClippingPlanes(1.0f, 100.0f);
		views.addView(view, 0);

		EXPECT_TRUE(views.getProjection(0)
					== views.getView(0).getProjectionMatrix());
	}

	TEST(ViewSetTest, DesktopSingleViewSingleSwapchain)
	{
		evan::ViewSet views;
		views.addView(utility::graphic::ViewF {}, 0);

		EXPECT_EQ(views.size(), 1u);
		EXPECT_EQ(views[0].swapchainIndex, 0u);
	}

	TEST(ViewSetTest, XrStereoTwoViewsTwoSwapchains)
	{
		evan::ViewSet views;
		views.addView(utility::graphic::ViewF {}, 0);
		views.addView(utility::graphic::ViewF {}, 1);

		EXPECT_EQ(views.size(), 2u);
		EXPECT_EQ(views[0].swapchainIndex, 0u);
		EXPECT_EQ(views[1].swapchainIndex, 1u);
	}

	TEST(ViewSetTest, MultiViewSharingSingleSwapchain)
	{
		evan::ViewSet views;
		views.addView(utility::graphic::ViewF {}, 0);
		views.addView(utility::graphic::ViewF {}, 0);

		EXPECT_EQ(views.size(), 2u);
		EXPECT_EQ(views[0].swapchainIndex, 0u);
		EXPECT_EQ(views[1].swapchainIndex, 0u);
	}

	TEST(MockSwapchainContextTest, MultiViewMappingThroughContext)
	{
		MockSwapchainContext context;
		context._viewSet.addView(utility::graphic::ViewF {}, 0);
		context._viewSet.addView(utility::graphic::ViewF {}, 1);

		EXPECT_EQ(context.getViewCount(), 2u);
		EXPECT_EQ(context.getViewSet().at(1).swapchainIndex, 1u);
		EXPECT_TRUE(context.getViewSet().getView(0).getProjectionMatrix()
					== context.getProjection(0));
	}
}	 // namespace xider::tests
