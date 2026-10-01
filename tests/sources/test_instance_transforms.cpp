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

#include <evan/Bounds.hpp>
#include <evan/EvanPlatform.hpp>
#include <evan/Frustum.hpp>

#include <utility/graphic/view.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <cstddef>
#include <initializer_list>

#include <gtest/gtest.h>

namespace xider::tests
{
	namespace
	{
		/**
		 * @brief Initial per-view capacity, the floor of the growth policy.
		 */
		constexpr std::size_t kSlab =
			static_cast<std::size_t>(MAX_INSTANCES_PER_VIEW);

		/**
		 * @brief Build a perspective view at the origin looking down -Z.
		 */
		utility::graphic::ViewF makeView()
		{
			utility::graphic::ViewF view;
			view.setClippingPlanes(0.1f, 100.0f);
			view.setPerspective(glm::radians(60.0f), 1.0f);
			return view;
		}

		/**
		 * @brief Build a frustum from a view using the same matrices as the
		 * renderer.
		 */
		evan::Frustum makeFrustum(const utility::graphic::ViewF &view)
		{
			return evan::Frustum::fromViewProjection(view.getProjectionMatrix()
													 * view.toViewMatrix());
		}

		/**
		 * @brief Build an axis-aligned box centered at (cx, cy, cz) with the
		 * given half extent on every axis.
		 */
		utility::math::AabbF makeBox(float cx, float cy, float cz, float half)
		{
			return utility::math::AabbF(
				utility::math::Vector<float, 3> { cx - half, cy - half,
												  cz - half },
				utility::math::Vector<float, 3> { cx + half, cy + half,
												  cz + half });
		}

		/**
		 * @brief Translation matrix helper.
		 */
		glm::mat4 translate(float x, float y, float z)
		{
			return glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
		}
	}	 // namespace

	TEST(InstanceCapacityTest, StartsAtTheInitialSlab)
	{
		EXPECT_EQ(nextInstanceCapacity(0, 0), kSlab);
	}

	TEST(InstanceCapacityTest, KeepsTheCurrentCapacityWhenItFits)
	{
		EXPECT_EQ(nextInstanceCapacity(1000, kSlab * 4), kSlab * 4);
		EXPECT_EQ(nextInstanceCapacity(kSlab * 4, kSlab * 4), kSlab * 4);
	}

	TEST(InstanceCapacityTest, GrowsGeometricallyPastTheSlab)
	{
		const std::size_t capacity = nextInstanceCapacity(kSlab + 1, 0);

		EXPECT_EQ(capacity, kSlab * 2);
		EXPECT_GE(capacity, kSlab + 1);
	}

	TEST(InstanceCapacityTest, NeverReturnsLessThanRequested)
	{
		const std::initializer_list<std::size_t> required { kSlab, kSlab + 1,
															kSlab * 5 + 7,
															100000 };

		for (const std::size_t count: required) {
			EXPECT_GE(nextInstanceCapacity(count, 0), count);
		}
	}

	TEST(InstanceCapacityTest, DoesNotShrinkBelowTheCurrentCapacity)
	{
		// A buffer already sized for a big scene must not fall back to the
		// slab when the next frame needs fewer transforms.
		EXPECT_EQ(nextInstanceCapacity(kSlab + 1, kSlab * 4), kSlab * 4);
		EXPECT_EQ(nextInstanceCapacity(kSlab * 8, kSlab * 4), kSlab * 8);
	}

	TEST(InstanceCapacityTest, IsStableAcrossCalls)
	{
		const std::size_t required = kSlab * 3 + 1;
		const std::size_t first	   = nextInstanceCapacity(required, 0);

		EXPECT_EQ(nextInstanceCapacity(required, first), first);
	}

	TEST(TransformedBoundsTest, IdentityKeepsTheLocalBounds)
	{
		const auto local = makeBox(1.0f, 2.0f, 3.0f, 1.0f);
		const auto world = evan::transformedBounds(local, glm::mat4(1.0f));

		EXPECT_FLOAT_EQ(world.getMin().x, local.getMin().x);
		EXPECT_FLOAT_EQ(world.getMin().y, local.getMin().y);
		EXPECT_FLOAT_EQ(world.getMax().z, local.getMax().z);
	}

	TEST(TransformedBoundsTest, EmptyBoundsStayEmpty)
	{
		EXPECT_TRUE(evan::transformedBounds(utility::math::AabbF {},
											translate(0.0f, 0.0f, -5.0f))
						.isEmpty());
	}

	TEST(TransformedBoundsTest, TranslationMovesTheBoxWithoutResizingIt)
	{
		const auto local = makeBox(0.0f, 0.0f, 0.0f, 1.0f);
		const auto world =
			evan::transformedBounds(local, translate(0.0f, 0.0f, -5.0f));

		EXPECT_FLOAT_EQ(world.center().z, -5.0f);
		EXPECT_FLOAT_EQ(world.extents().x, local.extents().x);
		EXPECT_FLOAT_EQ(world.extents().y, local.extents().y);
	}

	TEST(TransformedBoundsTest, ScaleGrowsTheExtents)
	{
		const auto local = makeBox(0.0f, 0.0f, 0.0f, 1.0f);
		const auto world = evan::transformedBounds(
			local, glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));

		EXPECT_FLOAT_EQ(world.extents().x, 4.0f);
		EXPECT_FLOAT_EQ(world.extents().y, 4.0f);
	}

	TEST(TransformedBoundsTest, RotationGrowsTheAxisAlignedExtents)
	{
		const auto local = makeBox(0.0f, 0.0f, 0.0f, 1.0f);
		const auto world = evan::transformedBounds(
			local,
			glm::rotate(glm::mat4(1.0f), glm::radians(45.0f),
						glm::vec3(0.0f, 0.0f, 1.0f)));

		// The box around a rotated box is larger than the rotated box itself.
		EXPECT_GT(world.extents().x, local.extents().x);
		EXPECT_NEAR(world.extents().x, 2.0f * std::sqrt(2.0f), 1.0e-5f);
	}

	TEST(TransformedBoundsTest, CullingFollowsTheTransformNotTheGeometry)
	{
		// Two objects share one local mesh but end up on opposite sides of the
		// camera. Culling on the local bounds would keep both, or drop both:
		// only the transformed bounds can tell them apart.
		const auto local   = makeBox(0.0f, 0.0f, 0.0f, 0.5f);
		const auto frustum = makeFrustum(makeView());

		const auto inFront =
			evan::transformedBounds(local, translate(0.0f, 0.0f, -5.0f));
		const auto behind =
			evan::transformedBounds(local, translate(0.0f, 0.0f, 5.0f));

		EXPECT_TRUE(frustum.intersects(inFront));
		EXPECT_FALSE(frustum.intersects(behind));
		EXPECT_NE(inFront.center().z, behind.center().z);
	}
}	 // namespace xider::tests
