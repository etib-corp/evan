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
#include <evan/ViewSet.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <vector>

namespace xider::tests
{
	/**
	 * @brief Minimal swapchain context exposing the protected format
	 * selection so it can be exercised with synthetic runtime values.
	 *
	 * The selection tests pass VK_NULL_HANDLE as the physical device, which
	 * skips the device support query and leaves the range validation and
	 * fallback logic under test.
	 */
	class TestableSwapchainContext: public evan::ASwapchainContext
	{
		public:
		using evan::ASwapchainContext::selectSwapchainFormat;

		void destroy(VkDevice) override
		{
		}

		void recreateSwapchain(const evan::DeviceContext &,
							   VkRenderPass) override
		{
		}

		VkSampleCountFlagBits getMsaaSamples() const override
		{
			return VK_SAMPLE_COUNT_1_BIT;
		}

		VkResult aquireImage(uint32_t, VkDevice, VkSemaphore, VkFence,
							 uint32_t &) override
		{
			return VK_SUCCESS;
		}

		void waitForImage(uint32_t) override
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

		private:
		evan::ViewSet _viewSet;
	};

	TEST(SwapchainFormatTest, SelectsPreferredFormat)
	{
		TestableSwapchainContext context;

		EXPECT_EQ(context.selectSwapchainFormat(VK_NULL_HANDLE,
												{ VK_FORMAT_R8G8B8A8_SRGB }),
				  VK_FORMAT_R8G8B8A8_SRGB);
	}

	TEST(SwapchainFormatTest, SelectsFirstListedPreferredFormat)
	{
		TestableSwapchainContext context;
		std::vector<int64_t> formats = { VK_FORMAT_B8G8R8A8_UNORM,
										 VK_FORMAT_R8G8B8A8_SRGB };

		EXPECT_EQ(context.selectSwapchainFormat(VK_NULL_HANDLE, formats),
				  VK_FORMAT_B8G8R8A8_UNORM);
	}

	TEST(SwapchainFormatTest, PrefersKnownFormatOverUnknownOne)
	{
		TestableSwapchainContext context;
		std::vector<int64_t> formats = {
			static_cast<int64_t>(VK_FORMAT_R16G16B16A16_SFLOAT),
			static_cast<int64_t>(VK_FORMAT_R8G8B8A8_UNORM)
		};

		EXPECT_EQ(context.selectSwapchainFormat(VK_NULL_HANDLE, formats),
				  VK_FORMAT_R8G8B8A8_UNORM);
	}

	TEST(SwapchainFormatTest, ReturnsPlausibleNonPreferredFormat)
	{
		TestableSwapchainContext context;
		std::vector<int64_t> formats = { static_cast<int64_t>(
			VK_FORMAT_R16G16B16A16_SFLOAT) };

		EXPECT_EQ(context.selectSwapchainFormat(VK_NULL_HANDLE, formats),
				  VK_FORMAT_R16G16B16A16_SFLOAT);
	}

	TEST(SwapchainFormatTest, EmptyListReturnsGuaranteedFallback)
	{
		TestableSwapchainContext context;

		EXPECT_EQ(context.selectSwapchainFormat(VK_NULL_HANDLE, {}),
				  VK_FORMAT_B8G8R8A8_UNORM);
	}

	TEST(SwapchainFormatTest, OutOfRangeValuesAreRejected)
	{
		TestableSwapchainContext context;
		std::vector<int64_t> formats = {
			-1, std::numeric_limits<int64_t>::min(),
			std::numeric_limits<int64_t>::max(),
			static_cast<int64_t>(VK_FORMAT_MAX_ENUM),
			static_cast<int64_t>(VK_FORMAT_UNDEFINED)
		};

		EXPECT_EQ(context.selectSwapchainFormat(VK_NULL_HANDLE, formats),
				  VK_FORMAT_B8G8R8A8_UNORM);
	}

	TEST(SwapchainFormatTest, InvalidValuesBeforeValidOneAreSkipped)
	{
		TestableSwapchainContext context;
		std::vector<int64_t> formats = {
			std::numeric_limits<int64_t>::max(), -42,
			static_cast<int64_t>(VK_FORMAT_R16G16B16A16_SFLOAT)
		};

		EXPECT_EQ(context.selectSwapchainFormat(VK_NULL_HANDLE, formats),
				  VK_FORMAT_R16G16B16A16_SFLOAT);
	}
}	 // namespace xider::tests
