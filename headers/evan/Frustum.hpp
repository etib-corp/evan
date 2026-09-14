/*
** ETIB PROJECT, 2026
** evan
** File description:
** Frustum
*/

#pragma once

#include <utility/math/aabb.hpp>

#include <glm/glm.hpp>

#include <array>
#include <cstddef>

namespace evan
{
	/**
	 * @brief A plane in 3D space in Hessian normal form.
	 *
	 * The plane normal points inward (toward the visible volume) and the
	 * distance is the signed offset such that a point @p p is inside when
	 * `dot(normal, p) + distance >= 0`.
	 */
	struct Plane {
		glm::vec3 normal { 0.0f };
		float distance = 0.0f;
	};

	/**
	 * @brief A view frustum represented by its six bounding planes.
	 *
	 * A Frustum is built once per view from the combined projection × view
	 * matrix and reused to reject meshes whose bounds are fully outside.
	 * Plane extraction uses the Gribb-Hartmann method and expects a
	 * Vulkan-style clip matrix (right-handed, 0..1 depth), matching the
	 * projection produced by utility::graphic::View::getProjectionMatrix().
	 */
	class Frustum
	{
		public:
		/**
		 * @brief Build a frustum from a combined projection × view matrix.
		 *
		 * @param viewProjection The combined clip-space matrix (proj * view).
		 * @return The frustum bounding the visible volume.
		 */
		static Frustum fromViewProjection(const glm::mat4 &viewProjection);

		/**
		 * @brief Test whether an axis-aligned bounding box intersects the
		 * frustum.
		 *
		 * A box is visible when it is inside or merely touching the frustum;
		 * only fully outside boxes are rejected. Empty boxes are always
		 * visible.
		 *
		 * @param box The bounding box to test.
		 * @return True when the box is (partially) inside the frustum.
		 */
		bool intersects(const utility::math::AabbF &box) const;

		/**
		 * @brief Test whether a bounding sphere intersects the frustum.
		 *
		 * @param center Center of the sphere.
		 * @param radius Radius of the sphere.
		 * @return True when the sphere is (partially) inside the frustum.
		 */
		bool intersects(const glm::vec3 &center, float radius) const;

		private:
		std::array<Plane, 6> _planes;
	};
}	 // namespace evan
