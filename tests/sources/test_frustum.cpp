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

#include <evan/Frustum.hpp>

#include <utility/graphic/view.hpp>

#include <gtest/gtest.h>

#include <glm/glm.hpp>

namespace xider::tests
{
	namespace
	{
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
			const glm::mat4 projection = view.getProjectionMatrix();
			const glm::mat4 viewMatrix = view.toViewMatrix();
			return evan::Frustum::fromViewProjection(projection * viewMatrix);
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
	}	 // namespace

	TEST(FrustumTest, BoxInFrontIsVisible)
	{
		const auto frustum = makeFrustum(makeView());

		EXPECT_TRUE(frustum.intersects(makeBox(0.0f, 0.0f, -5.0f, 1.0f)));
	}

	TEST(FrustumTest, BoxBehindCameraIsCulled)
	{
		const auto frustum = makeFrustum(makeView());

		EXPECT_FALSE(frustum.intersects(makeBox(0.0f, 0.0f, 5.0f, 1.0f)));
	}

	TEST(FrustumTest, BoxBeyondFarPlaneIsCulled)
	{
		const auto frustum = makeFrustum(makeView());

		EXPECT_FALSE(frustum.intersects(makeBox(0.0f, 0.0f, -200.0f, 1.0f)));
	}

	TEST(FrustumTest, BoxCloserThanNearPlaneIsCulled)
	{
		const auto frustum = makeFrustum(makeView());

		EXPECT_FALSE(
			frustum.intersects(makeBox(0.0f, 0.0f, -0.01f, 0.001f)));
	}

	TEST(FrustumTest, BoxFarToTheRightIsCulled)
	{
		const auto frustum = makeFrustum(makeView());

		EXPECT_FALSE(frustum.intersects(makeBox(100.0f, 0.0f, -5.0f, 0.5f)));
	}

	TEST(FrustumTest, EmptyBoxIsAlwaysVisible)
	{
		const auto frustum = makeFrustum(makeView());

		EXPECT_TRUE(frustum.intersects(utility::math::AabbF {}));
	}

	TEST(FrustumTest, SphereInFrontIsVisible)
	{
		const auto frustum = makeFrustum(makeView());

		EXPECT_TRUE(frustum.intersects(glm::vec3(0.0f, 0.0f, -5.0f), 1.0f));
	}

	TEST(FrustumTest, SphereBehindCameraIsCulled)
	{
		const auto frustum = makeFrustum(makeView());

		EXPECT_FALSE(frustum.intersects(glm::vec3(0.0f, 0.0f, 5.0f), 1.0f));
	}
}	 // namespace xider::tests
