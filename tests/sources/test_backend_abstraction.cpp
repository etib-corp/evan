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

#include <evan/ADeviceBackend.hpp>
#include <evan/ASwapchainContext.hpp>
#include <evan/ASwapchainImage.hpp>
#include <evan/CheckedCast.hpp>
#include <evan/IPlatform.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace xider::tests
{
	/**
	 * @brief Minimal backend that exercises the ADeviceBackend abstraction
	 * without any GLFW or OpenXR dependency. It sets the protected Vulkan
	 * handles directly to prove they are reachable by subclasses while no
	 * longer being public to consumers.
	 */
	class MockBackend: public evan::ADeviceBackend
	{
		public:
		MockBackend()
		{
			_device =
				reinterpret_cast<VkDevice>(static_cast<std::uintptr_t>(1u));
			_VkInstance =
				reinterpret_cast<VkInstance>(static_cast<std::uintptr_t>(2u));
			_physicalDevice = reinterpret_cast<VkPhysicalDevice>(
				static_cast<std::uintptr_t>(3u));
		}

		uint32_t countSwapchainFormats() const override
		{
			return 0;
		}

		std::vector<int64_t>
			enumerateSwapchainFormats(uint32_t) const override
		{
			return {};
		}

		evan::QueueFamilyIndices findQueueFamilies() override
		{
			return {};
		}

		bool preprocessFrame(evan::ASwapchainContext &) override
		{
			return true;
		}

		bool processFrame(VkPresentInfoKHR, evan::ASwapchainImage &) override
		{
			return true;
		}

		bool postprocessFrame(evan::ASwapchainContext &) override
		{
			return true;
		}

		protected:
		void createInstance(const evan::IPlatform &, const std::string &,
							evan::Version &) override
		{
		}

		void createLogicalDevice() override
		{
		}

		void pickPhysicalDevice() override
		{
		}
	};

	/**
	 * @brief A second, unrelated backend used to prove checkedCast rejects
	 * mismatched types.
	 */
	class MockOtherBackend: public evan::ADeviceBackend
	{
		public:
		uint32_t countSwapchainFormats() const override
		{
			return 0;
		}

		std::vector<int64_t>
			enumerateSwapchainFormats(uint32_t) const override
		{
			return {};
		}

		evan::QueueFamilyIndices findQueueFamilies() override
		{
			return {};
		}

		bool preprocessFrame(evan::ASwapchainContext &) override
		{
			return true;
		}

		bool processFrame(VkPresentInfoKHR, evan::ASwapchainImage &) override
		{
			return true;
		}

		bool postprocessFrame(evan::ASwapchainContext &) override
		{
			return true;
		}

		protected:
		void createInstance(const evan::IPlatform &, const std::string &,
							evan::Version &) override
		{
		}

		void createLogicalDevice() override
		{
		}

		void pickPhysicalDevice() override
		{
		}
	};

	/**
	 * @brief Platform mock that builds a MockBackend without requiring
	 * desktop (GLFW) or XR (OpenXR) specifics.
	 */
	class MockPlatform: public evan::IPlatform
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
			return std::make_shared<MockBackend>();
		}

		std::shared_ptr<evan::ASwapchainContext>
			createSwapchainContext(const evan::DeviceContext &) const override
		{
			return nullptr;
		}
	};

	TEST(BackendAbstractionTest, HandlesAreOnlyReachableThroughAccessors)
	{
		MockBackend backend;

		EXPECT_NE(backend.getDevice(), VK_NULL_HANDLE);
		EXPECT_NE(backend.getInstance(), VK_NULL_HANDLE);
		EXPECT_NE(backend.getPhysicalDevice(), VK_NULL_HANDLE);
	}

	TEST(BackendAbstractionTest, CheckedCastReturnsConcreteType)
	{
		MockBackend backend;
		evan::ADeviceBackend &base = backend;

		MockBackend &casted = evan::checkedCast<MockBackend>(base);

		EXPECT_EQ(&casted, &backend);
	}

	TEST(BackendAbstractionTest, CheckedCastThrowsOnMismatch)
	{
		MockBackend backend;
		evan::ADeviceBackend &base = backend;

		EXPECT_THROW((evan::checkedCast<MockOtherBackend>(base)),
					 std::logic_error);
	}

	TEST(BackendAbstractionTest, MockPlatformCreatesBackendWithoutGlfwOrXr)
	{
		MockPlatform platform;

		auto backend = platform.createDeviceBackend();

		ASSERT_NE(backend, nullptr);
		EXPECT_NE(backend->getDevice(), VK_NULL_HANDLE);
	}
}	 // namespace xider::tests
