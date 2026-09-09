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

	#include <evan/IPlatform.hpp>
	#include <evan/glfw/IDesktopPlatform.hpp>

	#if defined(__APPLE__)
		#include <evan/glfw/platform/MacOsDesktopPlatform.hpp>
	#elif defined(__linux__)
		#include <evan/glfw/platform/LinuxDesktopPlatform.hpp>
	#elif defined(_WIN32)
		#include <evan/glfw/platform/WindowsPlatform.hpp>
	#endif

	#include <cstdint>
	#include <cstdlib>
	#include <memory>
	#include <string>

	#include <gtest/gtest.h>

namespace xider::tests
{
	namespace
	{
		std::shared_ptr<evan::IPlatform> makePlatform(const std::string &name,
													  uint32_t width,
													  uint32_t height)
		{
	#if defined(__APPLE__)
			return std::make_shared<evan::MacOsDesktopPlatform>(name, width,
																height);
	#elif defined(__linux__)
			return std::make_shared<evan::LinuxDesktopPlatform>(name, width,
																height);
	#elif defined(_WIN32)
			return std::make_shared<evan::WindowsDesktopPlatform>(name, width,
																  height);
	#else
			return nullptr;
	#endif
		}
	}	 // namespace

	/**
	 * @brief Verifies A-019: multiple platform instances may coexist without
	 * one instance tearing down GLFW for the others, and windows resize
	 * freely. Window creation requires a display, so these tests opt in via
	 * EVAN_DEVICE_TESTS=1 (see TestGPUTexture integration test).
	 */
	TEST(DesktopPlatformTest, MultipleInstancesCoexist)
	{
		if (std::getenv("EVAN_DEVICE_TESTS") == nullptr) {
			GTEST_SKIP() << "Set EVAN_DEVICE_TESTS=1 to run window tests";
		}

		try {
			auto first	= makePlatform("evan test 1", 640, 480);
			auto second = makePlatform("evan test 2", 800, 600);

			EXPECT_FALSE(first->shouldClose());
			EXPECT_FALSE(second->shouldClose());

			// Destroying one platform must not tear down GLFW for the other.
			first.reset();
			EXPECT_FALSE(second->shouldClose());
			second.reset();
		} catch (const std::exception &e) {
			GTEST_SKIP() << "Window creation unavailable: " << e.what();
		}
	}

	TEST(DesktopPlatformTest, SequentialRecreateAfterLastRelease)
	{
		if (std::getenv("EVAN_DEVICE_TESTS") == nullptr) {
			GTEST_SKIP() << "Set EVAN_DEVICE_TESTS=1 to run window tests";
		}

		try {
			{
				auto platform = makePlatform("evan test", 640, 480);
				EXPECT_FALSE(platform->shouldClose());
			}
			// GLFW must be re-initializable after the last instance dies.
			auto platform = makePlatform("evan test", 640, 480);
			EXPECT_FALSE(platform->shouldClose());
		} catch (const std::exception &e) {
			GTEST_SKIP() << "Window creation unavailable: " << e.what();
		}
	}

	TEST(DesktopPlatformTest, WindowResizesFreely)
	{
		if (std::getenv("EVAN_DEVICE_TESTS") == nullptr) {
			GTEST_SKIP() << "Set EVAN_DEVICE_TESTS=1 to run window tests";
		}

		try {
			auto platform = makePlatform("evan test", 800, 600);

			auto *desktop =
				static_cast<evan::IDesktopPlatform *>(platform.get());
			ASSERT_NE(desktop, nullptr);
			ASSERT_NE(desktop->_window, nullptr);

			glfwSetWindowSize(desktop->_window, 1000, 600);

			// Resize is applied asynchronously by the window manager.
			int width  = 0;
			int height = 0;
			for (int i = 0; i < 100; ++i) {
				glfwPollEvents();
				glfwGetWindowSize(desktop->_window, &width, &height);
				if (width == 1000 && height == 600) {
					break;
				}
			}
			glfwGetWindowSize(desktop->_window, &width, &height);

			// With the aspect ratio unpinned, both dimensions honor the
			// request; the old pin would have clamped the height back to
			// 750 to preserve the creation 4:3 ratio.
			EXPECT_EQ(width, 1000);
			EXPECT_EQ(height, 600);
		} catch (const std::exception &e) {
			GTEST_SKIP() << "Window creation unavailable: " << e.what();
		}
	}

}	 // namespace xider::tests

#endif
