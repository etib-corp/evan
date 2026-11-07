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

#include <evan/GeometryCache.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace xider::tests
{
	namespace
	{
		/**
		 * @brief Builds a small triangle whose vertices can be varied.
		 */
		std::vector<evan::GPUVertex> makeVertices(float offset = 0.0f)
		{
			std::vector<evan::GPUVertex> vertices;

			for (std::size_t i = 0; i < 3; ++i) {
				evan::GPUVertex vertex;
				vertex.pos =
					glm::vec3(offset + static_cast<float>(i), 0.0f, 0.0f);
				vertex.color	= glm::vec3(1.0f, 0.0f, 0.0f);
				vertex.texCoord = glm::vec2(0.0f, 0.0f);
				vertices.push_back(vertex);
			}
			return vertices;
		}
	}	 // namespace

	TEST(GeometryCache, IdenticalContentSharesTheSameKey)
	{
		const std::vector<uint32_t> indices { 0u, 1u, 2u };

		EXPECT_EQ(evan::GeometryCache::computeKey(makeVertices(), indices, 7u),
				  evan::GeometryCache::computeKey(makeVertices(), indices, 7u));
	}

	TEST(GeometryCache, DifferentVerticesDoNotShareAKey)
	{
		const std::vector<uint32_t> indices { 0u, 1u, 2u };

		EXPECT_NE(
			evan::GeometryCache::computeKey(makeVertices(), indices, 7u),
			evan::GeometryCache::computeKey(makeVertices(1.0f), indices, 7u));
	}

	TEST(GeometryCache, DifferentIndicesDoNotShareAKey)
	{
		const auto vertices = makeVertices();

		EXPECT_NE(
			evan::GeometryCache::computeKey(vertices, { 0u, 1u, 2u }, 7u),
			evan::GeometryCache::computeKey(vertices, { 0u, 2u, 1u }, 7u));
	}

	TEST(GeometryCache, DifferentMaterialDoesNotShareAKey)
	{
		const auto vertices = makeVertices();
		const std::vector<uint32_t> indices { 0u, 1u, 2u };

		// A GPUMesh records the material it was built for, so two objects with
		// identical geometry but different materials must not share buffers.
		EXPECT_NE(evan::GeometryCache::computeKey(vertices, indices, 7u),
				  evan::GeometryCache::computeKey(vertices, indices, 8u));
	}

	TEST(GeometryCache, KeyDependsOnTheElementCounts)
	{
		const std::vector<uint32_t> indices { 0u, 1u, 2u };

		EXPECT_NE(evan::GeometryCache::computeKey(makeVertices(), indices, 7u),
				  evan::GeometryCache::computeKey({ makeVertices().front() },
												  indices, 7u));
	}

	TEST(GeometryCache, EmptyCacheHasNoEntry)
	{
		evan::GeometryCache cache;

		EXPECT_EQ(cache.size(), 0u);

		cache.clear();
		EXPECT_EQ(cache.size(), 0u);
	}
}	 // namespace xider::tests
