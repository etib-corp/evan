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

#include <evan/Scene.hpp>

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>
#include <vector>

namespace xider::tests
{
	/**
	 * @brief Verifies the Scene object-management API surface. These
	 * operations are CPU-side map manipulations and do not require a live
	 * Vulkan device, making them suitable for unit testing.
	 */
	TEST(SceneTest, AddObjectReturnsAssignedId)
	{
		evan::Scene scene;

		EXPECT_EQ(scene.addObject(42, nullptr), 42u);
		EXPECT_EQ(scene.addObject(7, nullptr), 7u);
	}

	TEST(SceneTest, RemoveExistingObject)
	{
		evan::Scene scene;

		scene.addObject(1, nullptr);
		scene.addObject(2, nullptr);

		EXPECT_TRUE(scene.removeObject(1));
		EXPECT_FALSE(scene.removeObject(1));
		EXPECT_FALSE(scene.removeObject(999));
	}

	TEST(SceneTest, MeshesEmptyForEmptyScene)
	{
		evan::Scene scene;

		EXPECT_TRUE(scene.getMeshes().empty());
	}
	/**
	 * @brief Ensures getMeshes() returns a constant reference to the Scene's
	 * cached mesh vector, so repeated per-frame calls do not allocate.
	 */
	TEST(SceneTest, GetMeshesReturnsConstReference)
	{
		static_assert(
			std::is_same_v<
				decltype(std::declval<const evan::Scene &>().getMeshes()),
				const std::vector<std::shared_ptr<evan::GPUMesh>> &>,
			"Scene::getMeshes() must return a const reference");

		evan::Scene scene;

		EXPECT_TRUE(scene.getMeshes().empty());
	}

	/**
	 * @brief Ensures consecutive getMeshes() calls return the same cached
	 * vector, i.e. no per-call allocation.
	 */
	TEST(SceneTest, GetMeshesIsCached)
	{
		evan::Scene scene;

		const auto &first  = scene.getMeshes();
		const auto &second = scene.getMeshes();

		EXPECT_EQ(&first, &second);
	}

	/**
	 * @brief Ensures adding an object invalidates the mesh cache. Null
	 * objects are skipped when flattening, but the invalidation path is
	 * still exercised.
	 */
	TEST(SceneTest, AddObjectInvalidatesMeshesCache)
	{
		evan::Scene scene;

		EXPECT_TRUE(scene.getMeshes().empty());

		scene.addObject(1, nullptr);

		EXPECT_TRUE(scene.getMeshes().empty());
	}

	/**
	 * @brief Ensures removing an object invalidates the mesh cache.
	 */
	TEST(SceneTest, RemoveObjectInvalidatesMeshesCache)
	{
		evan::Scene scene;

		scene.addObject(1, nullptr);
		EXPECT_TRUE(scene.getMeshes().empty());

		EXPECT_TRUE(scene.removeObject(1));
		EXPECT_TRUE(scene.getMeshes().empty());
	}
}	 // namespace xider::tests
