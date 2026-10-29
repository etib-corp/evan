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

#ifdef __OPENXR__

	#include <evan/openxr/XrDeviceBackend.hpp>

	#include <gtest/gtest.h>

	#include <array>
	#include <span>

namespace xider::tests
{
	TEST(EnvironmentBlendModeTest, PrefersOpaque)
	{
		const std::array modes {
			XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND,
			XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
		};

		EXPECT_EQ(
			evan::XrDeviceBackend::selectDefaultEnvironmentBlendMode(modes),
			XR_ENVIRONMENT_BLEND_MODE_OPAQUE);
	}

	TEST(EnvironmentBlendModeTest, FallsBackToFirstWithoutOpaque)
	{
		const std::array modes {
			XR_ENVIRONMENT_BLEND_MODE_ADDITIVE,
			XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND,
		};

		EXPECT_EQ(
			evan::XrDeviceBackend::selectDefaultEnvironmentBlendMode(modes),
			XR_ENVIRONMENT_BLEND_MODE_ADDITIVE);
	}

	TEST(EnvironmentBlendModeTest, EmptyFallsBackToOpaque)
	{
		const std::span<const XrEnvironmentBlendMode> modes;

		EXPECT_EQ(
			evan::XrDeviceBackend::selectDefaultEnvironmentBlendMode(modes),
			XR_ENVIRONMENT_BLEND_MODE_OPAQUE);
	}

	TEST(EnvironmentBlendModeTest, SelectsSingleOpaqueMode)
	{
		const std::array modes { XR_ENVIRONMENT_BLEND_MODE_OPAQUE };

		EXPECT_EQ(
			evan::XrDeviceBackend::selectDefaultEnvironmentBlendMode(modes),
			XR_ENVIRONMENT_BLEND_MODE_OPAQUE);
	}
}	 // namespace xider::tests

#endif	  // __OPENXR__
