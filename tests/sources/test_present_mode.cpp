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

#include <gtest/gtest.h>

namespace xider::tests
{
	/**
	 * @brief Without an override the swapchain keeps vsync pacing.
	 */
	TEST(PresentModeTest, MissingOrEmptyValueKeepsFifo)
	{
		EXPECT_EQ(::presentModeFromValue(nullptr), PresentModePreference::Fifo);
		EXPECT_EQ(::presentModeFromValue(""), PresentModePreference::Fifo);
		EXPECT_EQ(::presentModeFromValue("0"), PresentModePreference::Fifo);
	}

	/**
	 * @brief Explicit FIFO and its aliases keep vsync pacing.
	 */
	TEST(PresentModeTest, FifoAliasesAreAccepted)
	{
		EXPECT_EQ(::presentModeFromValue("fifo"), PresentModePreference::Fifo);
		EXPECT_EQ(::presentModeFromValue("vsync"), PresentModePreference::Fifo);
	}

	/**
	 * @brief The uncapped modes are selectable by name.
	 */
	TEST(PresentModeTest, UncappedModesAreSelectable)
	{
		EXPECT_EQ(::presentModeFromValue("mailbox"),
				  PresentModePreference::Mailbox);
		EXPECT_EQ(::presentModeFromValue("immediate"),
				  PresentModePreference::Immediate);
		EXPECT_EQ(::presentModeFromValue("nowait"),
				  PresentModePreference::Immediate);
		EXPECT_EQ(::presentModeFromValue("auto"), PresentModePreference::Auto);
	}

	/**
	 * @brief An unrecognised value must not silently uncap the frame loop.
	 */
	TEST(PresentModeTest, UnknownValueFallsBackToFifo)
	{
		EXPECT_EQ(::presentModeFromValue("triple"),
				  PresentModePreference::Fifo);
		EXPECT_EQ(::presentModeFromValue("MAILBOX"),
				  PresentModePreference::Fifo);
	}
}	 // namespace xider::tests
