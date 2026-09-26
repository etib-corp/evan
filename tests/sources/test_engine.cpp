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

#include "test_engine.hpp"

#include <evan/Renderer.hpp>
#include <utility/graphic/scissor.hpp>

namespace xider::tests
{
	void TestEngine::SetUp(void)
	{
	}

	void TestEngine::TearDown(void)
	{
	}

	TEST_F(TestEngine, SampleTest)
	{
		// This is a sample test case. Replace it with actual tests for the
		// Engine class.
		EXPECT_TRUE(true);
	}

	TEST_F(TestEngine, ClampScissorKeepsRectInsideExtent)
	{
		const VkExtent2D extent { 100, 50 };
		const VkRect2D result = evan::Renderer::clampScissor(
			utility::graphic::ScissorRect { 10.0f, 5.0f, 20.0f, 10.0f },
			extent);

		EXPECT_EQ(result.offset.x, 10);
		EXPECT_EQ(result.offset.y, 5);
		EXPECT_EQ(result.extent.width, 20u);
		EXPECT_EQ(result.extent.height, 10u);
	}

	TEST_F(TestEngine, ClampScissorClampsNegativeOrigin)
	{
		const VkExtent2D extent { 100, 50 };
		const VkRect2D result = evan::Renderer::clampScissor(
			utility::graphic::ScissorRect { -10.0f, -10.0f, 40.0f, 40.0f },
			extent);

		EXPECT_EQ(result.offset.x, 0);
		EXPECT_EQ(result.offset.y, 0);
		EXPECT_EQ(result.extent.width, 30u);
		EXPECT_EQ(result.extent.height, 30u);
	}

	TEST_F(TestEngine, ClampScissorClampsOverflow)
	{
		const VkExtent2D extent { 100, 50 };
		const VkRect2D result = evan::Renderer::clampScissor(
			utility::graphic::ScissorRect { 90.0f, 40.0f, 50.0f, 50.0f },
			extent);

		EXPECT_EQ(result.offset.x, 90);
		EXPECT_EQ(result.offset.y, 40);
		EXPECT_EQ(result.extent.width, 10u);
		EXPECT_EQ(result.extent.height, 10u);
	}

	TEST_F(TestEngine, ClampScissorHandlesNonPositiveSize)
	{
		const VkExtent2D extent { 100, 50 };
		const VkRect2D result = evan::Renderer::clampScissor(
			utility::graphic::ScissorRect { 0.0f, 0.0f, -5.0f, -5.0f }, extent);

		EXPECT_EQ(result.extent.width, 0u);
		EXPECT_EQ(result.extent.height, 0u);
	}

}	 // namespace xider::tests