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

#include <utility/graphic/view.hpp>

#include <gtest/gtest.h>

#include <numbers>

namespace xider::tests
{
	TEST(SwapchainViewTest, UpdateViewForExtentPreservesVerticalFovAndPose)
	{
		utility::graphic::ViewF view;
		view.setClippingPlanes(1.0f, 100.0f);
		view.setPerspective(1.0f, 16.0f / 9.0f);
		const auto poseBefore = view.getPose();

		VkExtent2D extent { 1280, 720 };
		evan::ASwapchainContext::updateViewForExtent(view, extent);

		EXPECT_FLOAT_EQ(view.getVerticalFovRadians(), 1.0f);
		EXPECT_EQ(view.getPose(), poseBefore);
		EXPECT_FLOAT_EQ(view.getViewportSize().x, 1280.0f);
		EXPECT_FLOAT_EQ(view.getViewportSize().y, 720.0f);
	}

	TEST(SwapchainViewTest, UpdateViewForExtentAppliesDefaultFovWhenUnset)
	{
		utility::graphic::ViewF view;
		VkExtent2D extent { 800, 600 };

		evan::ASwapchainContext::updateViewForExtent(view, extent);

		EXPECT_FLOAT_EQ(view.getVerticalFovRadians(),
						std::numbers::pi_v<float> * 0.5f);
		EXPECT_FLOAT_EQ(view.getViewportSize().x, 800.0f);
		EXPECT_FLOAT_EQ(view.getViewportSize().y, 600.0f);
	}

	TEST(SwapchainViewTest, UpdateViewForExtentUsesFullExtentNotHalf)
	{
		utility::graphic::ViewF view;
		VkExtent2D extent { 1920, 1080 };

		evan::ASwapchainContext::updateViewForExtent(view, extent);

		EXPECT_FLOAT_EQ(view.getViewportSize().x, 1920.0f);
		EXPECT_FLOAT_EQ(view.getViewportSize().y, 1080.0f);
	}
}	 // namespace xider::tests
