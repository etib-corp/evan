/*
** ETIB PROJECT, 2026
** evan
** File description:
** Mesh
*/

#pragma once

#include "EvanPlatform.hpp"

#include "Vertex.hpp"

#include <string>

namespace evan
{
	/**
	 * @brief A structure representing a 3D mesh.
	 *
	 * This structure contains the vertices, indices, and material path for a 3D
	 * mesh. It is used to store the data necessary to render a 3D object in a
	 * graphics application.
	 *
	 * - `vertices`: A vector of `Vertex` objects representing the vertices of
	 * the mesh. Each vertex contains position, color, and texture coordinate
	 * information.
	 *
	 * - `indices`: A vector of `uint32_t` values representing the indices of
	 * the vertices in the mesh. These indices define how the vertices are
	 * connected to form triangles for rendering.
	 *
	 *  - `materialPath`: A string representing the file path to the material
	 * associated with the mesh. This material may contain information about the
	 * texture, color, and other properties used to render the mesh.
	 *
	 * This structure is essential for representing 3D models in a graphics
	 * application, allowing for efficient storage and rendering of complex
	 * objects.
	 */
	struct Mesh {
		/**
		 * @brief A vector of `Vertex` objects representing the vertices of the
		 * mesh. Each vertex contains position, color, and texture coordinate
		 * information.
		 */
		std::vector<Vertex> vertices;

		/**
		 * @brief A vector of `uint32_t` values representing the indices of the
		 * vertices in the mesh. These indices define how the vertices are
		 * connected to form triangles for rendering.
		 */
		std::vector<uint32_t> indices;

		/**
		 * @brief A string representing the file path to the material associated
		 * with the mesh. This material may contain information about the
		 * texture, color, and other properties used to render the mesh.
		 */
		std::string materialPath;
	};
}	 // namespace evan
