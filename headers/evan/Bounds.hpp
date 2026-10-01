/*
** ETIB PROJECT, 2026
** evan
** File description:
** Bounds
*/

#pragma once

#include <utility/math/aabb.hpp>

#include <glm/glm.hpp>

namespace evan
{
	/**
	 * @brief Computes the world-space bounds of a local-space bounding box.
	 *
	 * Meshes store local-space geometry plus a model matrix, so the box built
	 * from their vertices is a local box. Culling and depth sorting must apply
	 * the model matrix first: otherwise every mesh would be tested against a
	 * box sitting at the mesh's own origin, and frustum or distance culling
	 * would reject visible objects.
	 *
	 * The returned box is the axis-aligned box of the eight transformed
	 * corners, that is the tightest axis-aligned box containing the
	 * transformed box.
	 *
	 * An identity transform returns @p localBounds unchanged, so meshes
	 * without a transform keep the exact bounds computed from their vertices.
	 *
	 * @param localBounds The bounds of the mesh in its own space.
	 * @param transform The model matrix of the mesh.
	 * @return The bounds in world space, empty when @p localBounds is empty.
	 */
	inline utility::math::AabbF
		transformedBounds(const utility::math::AabbF &localBounds,
						  const glm::mat4 &transform)
	{
		if (localBounds.isEmpty() || transform == glm::mat4(1.0f)) {
			return localBounds;
		}

		const auto min = localBounds.getMin();
		const auto max = localBounds.getMax();

		utility::math::AabbF worldBounds;
		for (unsigned int corner = 0; corner < 8; ++corner) {
			const glm::vec4 position((corner & 1u) != 0u ? max.x : min.x,
									 (corner & 2u) != 0u ? max.y : min.y,
									 (corner & 4u) != 0u ? max.z : min.z, 1.0f);
			worldBounds.include(utility::math::Vector<float, 3>(
				glm::vec3(transform * position)));
		}
		return worldBounds;
	}
}	 // namespace evan
