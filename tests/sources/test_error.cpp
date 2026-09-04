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

#include <evan/Error.hpp>

#ifdef __OPENXR__
	#include <evan/openxr/XrError.hpp>
#endif

#include <gtest/gtest.h>

namespace xider::tests
{
	TEST(ErrorTest, VkResultMappings)
	{
		EXPECT_EQ(evan::mapVkResult(VK_SUCCESS), evan::Error::Ok);
		EXPECT_EQ(evan::mapVkResult(VK_SUBOPTIMAL_KHR),
				  evan::Error::Suboptimal);
		EXPECT_EQ(evan::mapVkResult(VK_ERROR_OUT_OF_DATE_KHR),
				  evan::Error::SwapchainOutOfDate);
		EXPECT_EQ(evan::mapVkResult(VK_ERROR_SURFACE_LOST_KHR),
				  evan::Error::SwapchainOutOfDate);
		EXPECT_EQ(evan::mapVkResult(VK_NOT_READY), evan::Error::NotReady);
		EXPECT_EQ(evan::mapVkResult(VK_TIMEOUT), evan::Error::NotReady);
		EXPECT_EQ(evan::mapVkResult(VK_ERROR_DEVICE_LOST),
				  evan::Error::DeviceLost);
		EXPECT_EQ(evan::mapVkResult(VK_ERROR_OUT_OF_HOST_MEMORY),
				  evan::Error::OutOfMemory);
		EXPECT_EQ(evan::mapVkResult(VK_ERROR_OUT_OF_DEVICE_MEMORY),
				  evan::Error::OutOfMemory);
	}

	TEST(ErrorTest, Classification)
	{
		EXPECT_TRUE(evan::isOk(evan::Error::Ok));
		EXPECT_FALSE(evan::isOk(evan::Error::SwapchainOutOfDate));

		EXPECT_TRUE(evan::isFatal(evan::Error::DeviceLost));
		EXPECT_TRUE(evan::isFatal(evan::Error::RuntimeLost));
		EXPECT_TRUE(evan::isFatal(evan::Error::OutOfMemory));
		EXPECT_TRUE(evan::isFatal(evan::Error::RuntimeError));

		EXPECT_FALSE(evan::isFatal(evan::Error::Ok));
		EXPECT_FALSE(evan::isFatal(evan::Error::Suboptimal));
		EXPECT_FALSE(evan::isFatal(evan::Error::SwapchainOutOfDate));
		EXPECT_FALSE(evan::isFatal(evan::Error::NotReady));

		EXPECT_TRUE(evan::isRecoverable(evan::Error::SwapchainOutOfDate));
		EXPECT_TRUE(evan::isRecoverable(evan::Error::Suboptimal));
		EXPECT_TRUE(evan::isRecoverable(evan::Error::NotReady));
		EXPECT_FALSE(evan::isRecoverable(evan::Error::Ok));
		EXPECT_FALSE(evan::isRecoverable(evan::Error::DeviceLost));
	}

	// Synthetic recovery injection: a renderer must distinguish a recoverable
	// swapchain-out-of-date from a fatal device-lost.
	TEST(ErrorTest, SyntheticOutOfDateVsDeviceLost)
	{
		evan::Error outOfDate = evan::mapVkResult(VK_ERROR_OUT_OF_DATE_KHR);
		evan::Error deviceLost = evan::mapVkResult(VK_ERROR_DEVICE_LOST);

		EXPECT_TRUE(evan::isRecoverable(outOfDate));
		EXPECT_TRUE(evan::isFatal(deviceLost));
		EXPECT_NE(outOfDate, deviceLost);
	}

#ifdef __OPENXR__
	TEST(ErrorTest, XrResultMappings)
	{
		EXPECT_EQ(evan::mapXrResult(XR_SUCCESS), evan::Error::Ok);
		EXPECT_EQ(evan::mapXrResult(XR_ERROR_SESSION_LOST),
				  evan::Error::RuntimeLost);
		EXPECT_EQ(evan::mapXrResult(XR_ERROR_INSTANCE_LOST),
				  evan::Error::RuntimeLost);
		EXPECT_EQ(evan::mapXrResult(XR_ERROR_SESSION_NOT_RUNNING),
				  evan::Error::NotReady);
		EXPECT_EQ(evan::mapXrResult(XR_ERROR_TIME_INVALID),
				  evan::Error::NotReady);
	}

	TEST(ErrorTest, SyntheticSessionLossPending)
	{
		evan::Error loss =
			evan::mapSessionState(XR_SESSION_STATE_LOSS_PENDING);
		EXPECT_EQ(loss, evan::Error::RuntimeLost);
		EXPECT_TRUE(evan::isFatal(loss));

		evan::Error exiting = evan::mapSessionState(XR_SESSION_STATE_EXITING);
		EXPECT_EQ(exiting, evan::Error::Ok);
	}
#endif
}	 // namespace xider::tests
