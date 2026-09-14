/*
** ETIB PROJECT, 2026
** evan
** File description:
** Frustum
*/

#include "evan/Frustum.hpp"

evan::Frustum evan::Frustum::fromViewProjection(const glm::mat4 &viewProjection)
{
	Frustum frustum;

	// Gribb-Hartmann plane extraction for a right-handed, 0..1 depth clip
	// matrix (Vulkan convention). GLM is column-major (m[col][row]), so each
	// mathematical row is gathered manually; the plane vector is (a, b, c, d)
	// where (a, b, c) is the inward-pointing normal and d the signed offset.
	const auto &m = viewProjection;
	const glm::vec4 row0(m[0][0], m[1][0], m[2][0], m[3][0]);
	const glm::vec4 row1(m[0][1], m[1][1], m[2][1], m[3][1]);
	const glm::vec4 row2(m[0][2], m[1][2], m[2][2], m[3][2]);
	const glm::vec4 row3(m[0][3], m[1][3], m[2][3], m[3][3]);

	// Left:   x + w >= 0  ->  row0 + row3
	// Right:  w - x >= 0  ->  row3 - row0
	// Bottom: y + w >= 0  ->  row1 + row3
	// Top:    w - y >= 0  ->  row3 - row1
	// Near:   z >= 0      ->  row2          (0..1 depth)
	// Far:    w - z >= 0  ->  row3 - row2
	frustum._planes[0].normal   = glm::vec3(row0 + row3);
	frustum._planes[0].distance = row0.w + row3.w;
	frustum._planes[1].normal   = glm::vec3(row3 - row0);
	frustum._planes[1].distance = row3.w - row0.w;
	frustum._planes[2].normal   = glm::vec3(row1 + row3);
	frustum._planes[2].distance = row1.w + row3.w;
	frustum._planes[3].normal   = glm::vec3(row3 - row1);
	frustum._planes[3].distance = row3.w - row1.w;
	frustum._planes[4].normal   = glm::vec3(row2);
	frustum._planes[4].distance = row2.w;
	frustum._planes[5].normal   = glm::vec3(row3 - row2);
	frustum._planes[5].distance = row3.w - row2.w;

	// Normalize each plane so signed distances are in world units.
	for (Plane &plane : frustum._planes) {
		const float length = glm::length(plane.normal);
		if (length > 0.0f) {
			plane.normal /= length;
			plane.distance /= length;
		}
	}

	return frustum;
}

bool evan::Frustum::intersects(const utility::math::AabbF &box) const
{
	if (box.isEmpty()) {
		return true;
	}

	for (const Plane &plane : _planes) {
		// The positive vertex is the box corner farthest along the plane
		// normal. If even that corner lies outside, the whole box is outside.
		const glm::vec3 positive {
			plane.normal.x >= 0.0f ? box.getMax().x : box.getMin().x,
			plane.normal.y >= 0.0f ? box.getMax().y : box.getMin().y,
			plane.normal.z >= 0.0f ? box.getMax().z : box.getMin().z,
		};
		if (glm::dot(plane.normal, positive) + plane.distance < 0.0f) {
			return false;
		}
	}

	return true;
}

bool evan::Frustum::intersects(const glm::vec3 &center, float radius) const
{
	for (const Plane &plane : _planes) {
		if (glm::dot(plane.normal, center) + plane.distance < -radius) {
			return false;
		}
	}

	return true;
}
