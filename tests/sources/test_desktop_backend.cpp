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

#ifdef __GLFW__

	#include <evan/ADeviceBackend.hpp>
	#include <evan/ASwapchainContext.hpp>
	#include <evan/CheckedCast.hpp>
	#include <evan/IPlatform.hpp>
	#include <evan/glfw/DesktopBackend.hpp>

	#include <gtest/gtest.h>

	#include <memory>
	#include <string>
	#include <vector>

namespace xider::tests
{
	/**
	 * @brief A platform that is *not* an IDesktopPlatform.
	 *
	 * Used to prove that DesktopBackend rejects a mismatched platform type
	 * loudly (via checkedCast) instead of dereferencing a null pointer.
	 */
	class NonDesktopPlatform: public evan::IPlatform
	{
		public:
		std::vector<std::string> getRequiredInstanceExtensions() const override
		{
			return {};
		}

		bool shouldClose() const override
		{
			return false;
		}

		std::vector<std::shared_ptr<utility::event::Event>>
			pollEvents(evan::ADeviceBackend &) override
		{
			return {};
		}

		std::shared_ptr<evan::ADeviceBackend>
			createDeviceBackend() const override
		{
			return nullptr;
		}

		std::shared_ptr<evan::ASwapchainContext>
			createSwapchainContext(const evan::DeviceContext &) const override
		{
			return nullptr;
		}
	};

	TEST(DesktopBackendTest, ConstructorRejectsNonDesktopPlatform)
	{
		NonDesktopPlatform platform;

		EXPECT_THROW(evan::DesktopBackend { platform }, std::logic_error);
	}
}	 // namespace xider::tests

#endif	  // __GLFW__
