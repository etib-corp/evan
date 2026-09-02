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

#include "test_gputexture.hpp"

#include <cstdlib>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include <evan/DeviceContext.hpp>
#include <utility/graphic/texture.hpp>

#ifdef __GLFW__
	#if defined(__APPLE__)
		#include <evan/glfw/platform/MacOsDesktopPlatform.hpp>
	#elif defined(__linux__)
		#include <evan/glfw/platform/LinuxDesktopPlatform.hpp>
	#elif defined(_WIN32)
		#include <evan/glfw/platform/WindowsPlatform.hpp>
	#endif
#endif

namespace xider::tests
{
	void TestGPUTexture::SetUp(void)
	{
	}

	void TestGPUTexture::TearDown(void)
	{
	}

	TEST_F(TestGPUTexture, ComputeMipLevelsMatchesDimensions)
	{
		EXPECT_EQ(evan::GPUTexture::computeMipLevels(1, 1), 1u);
		EXPECT_EQ(evan::GPUTexture::computeMipLevels(2, 1), 2u);
		EXPECT_EQ(evan::GPUTexture::computeMipLevels(64, 64), 7u);
		EXPECT_EQ(evan::GPUTexture::computeMipLevels(512, 512), 10u);
		EXPECT_EQ(evan::GPUTexture::computeMipLevels(1024, 1024), 11u);
		EXPECT_EQ(evan::GPUTexture::computeMipLevels(800, 600), 10u);
	}

	TEST_F(TestGPUTexture, ComputeMaxLodIsZeroBased)
	{
		EXPECT_FLOAT_EQ(evan::GPUTexture::computeMaxLod(0), 0.0f);
		EXPECT_FLOAT_EQ(evan::GPUTexture::computeMaxLod(1), 0.0f);
		EXPECT_FLOAT_EQ(evan::GPUTexture::computeMaxLod(2), 1.0f);
		EXPECT_FLOAT_EQ(evan::GPUTexture::computeMaxLod(11), 10.0f);
	}

	TEST_F(TestGPUTexture, AllGeneratedMipLevelsAreSampleable)
	{
		const std::vector<std::pair<uint32_t, uint32_t>> dimensions = {
			{ 1, 1 },	  { 2, 1 },		 { 64, 64 },
			{ 128, 256 }, { 800, 600 },	 { 1024, 1024 },
			{ 2048, 512 },
		};

		for (const auto &[width, height]: dimensions) {
			const uint32_t mipLevels =
				evan::GPUTexture::computeMipLevels(width, height);
			const float maxLod = evan::GPUTexture::computeMaxLod(mipLevels);

			EXPECT_GT(mipLevels, 0u);
			// The sampler's [0, maxLod] range must cover every mip level
			// index.
			for (uint32_t level = 0; level < mipLevels; ++level) {
				EXPECT_LE(static_cast<float>(level), maxLod);
			}
			// The highest sampleable LOD equals the last mip level index.
			EXPECT_FLOAT_EQ(maxLod, static_cast<float>(mipLevels - 1));
		}
	}

#ifdef __GLFW__
	TEST_F(TestGPUTexture, DeviceIntegrationExposesAllMipLevels)
	{
		// Opt-in: creating a real device requires a windowing surface, which
		// is not available in headless CI environments. Run with
		// EVAN_DEVICE_TESTS=1 on a machine with a display.
		if (std::getenv("EVAN_DEVICE_TESTS") == nullptr) {
			GTEST_SKIP() << "Set EVAN_DEVICE_TESTS=1 to run device tests";
		}

		VkApplicationInfo appInfo {};
		appInfo.sType	   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo {};
		createInfo.sType			 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		VkInstance probe = VK_NULL_HANDLE;
		if (vkCreateInstance(&createInfo, nullptr, &probe) != VK_SUCCESS) {
			GTEST_SKIP() << "Vulkan runtime not available";
		}
		vkDestroyInstance(probe, nullptr);

		try {
			#if defined(__APPLE__)
			auto platform = std::make_shared<evan::MacOsDesktopPlatform>(
				"evan test", 64, 64);
			#elif defined(__linux__)
			auto platform = std::make_shared<evan::LinuxDesktopPlatform>(
				"evan test", 64, 64);
			#elif defined(_WIN32)
			auto platform = std::make_shared<evan::WindowsDesktopPlatform>(
				"evan test", 64, 64);
			#endif

			evan::DeviceContext deviceContext(*platform);
			utility::graphic::Texture texture(
				64, 64, utility::graphic::Texture::TextureType::Albedo);
			evan::GPUTexture gpuTexture(deviceContext, texture);

			EXPECT_EQ(gpuTexture.getMipLevels(),
					  evan::GPUTexture::computeMipLevels(64, 64));
			EXPECT_NE(gpuTexture.sampler,
					  static_cast<VkSampler>(VK_NULL_HANDLE));
			EXPECT_NE(gpuTexture.view,
					  static_cast<VkImageView>(VK_NULL_HANDLE));
		} catch (const std::exception &e) {
			GTEST_SKIP() << "Unable to create device context: " << e.what();
		}
	}
#endif

}	 // namespace xider::tests
