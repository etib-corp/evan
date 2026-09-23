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

#include <cstdint>
#include <utility>
#include <vector>

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
			{ 1, 1 },	  { 2, 1 },		  { 64, 64 },	 { 128, 256 },
			{ 800, 600 }, { 1024, 1024 }, { 2048, 512 },
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

}	 // namespace xider::tests
