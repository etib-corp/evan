/*
** ETIB PROJECT, 2025
** evan
** File description:
** GPUVertex
*/

#ifndef GPUVERTEX_HPP_
#define GPUVERTEX_HPP_

#include "evan/EvanPlatform.hpp"

#include <utility/graphic/vertex.hpp>

#include <glm/glm.hpp>

namespace evan
{
	/**
	 * @brief Per-instance data fed to the vertex shader.
	 *
	 * One GPUInstance is consumed per drawn instance. The model matrix
	 * replaces the CPU-baked world transform when instancing is enabled,
	 * letting identical meshes share geometry and differ only by transform.
	 */
	struct GPUInstance {
		glm::mat4 model = glm::mat4(1.0f);
	};

	/**
	 * @brief GPUVertex structure.
	 *
	 * This structure represents a vertex in the 3D space. It contains the
	 * position, color, and texture coordinates of the vertex. It is used to
	 * define the vertex data for rendering.
	 *
	 */
	struct GPUVertex {
		glm::vec3 pos;		///< Position of the vertex
		glm::vec3 color;	///< Color of the vertex    (TODO: change to vec4
							///< for alpha)

		/**
		 * @brief Texture coordinates of the vertex.
		 */
		glm::vec2 texCoord;

		/**
		 * @brief Get the binding description for the vertex.
		 *
		 * This function returns the binding description for the vertex,
		 * which specifies how the vertex data is laid out in memory.
		 *
		 * @return VkVertexInputBindingDescription The binding description
		 *         for the vertex.
		 *
		 */
		static VkVertexInputBindingDescription getBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription {};
			bindingDescription.binding	 = 0;
			bindingDescription.stride	 = sizeof(GPUVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		/**
		 * @brief Get the attribute descriptions for the vertex.
		 *
		 * This function returns an array of attribute descriptions for the
		 * vertex, which specify the format and offset of each attribute in
		 * the vertex data.
		 *
		 * @return std::array<VkVertexInputAttributeDescription, 3> The
		 *         attribute descriptions for the vertex.
		 *
		 */
		static std::array<VkVertexInputAttributeDescription, 3>
			getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 3>
				attributeDescriptions {};

			attributeDescriptions[0].binding  = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format	  = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset	  = offsetof(GPUVertex, pos);

			attributeDescriptions[1].binding  = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format	  = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset	  = offsetof(GPUVertex, color);

			attributeDescriptions[2].binding  = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format	  = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset	  = offsetof(GPUVertex, texCoord);

			return attributeDescriptions;
		}

		/**
		 * @brief Get the binding description for per-instance data.
		 *
		 * Binding 1 carries one 4x4 model matrix per instance at an instance
		 * input rate, so identical meshes can be drawn with a single
		 * vkCmdDrawIndexed(instanceCount = N).
		 *
		 * @return VkVertexInputBindingDescription for the instance data.
		 */
		static VkVertexInputBindingDescription getInstanceBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription {};
			bindingDescription.binding	 = 1;
			bindingDescription.stride	 = sizeof(GPUInstance);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

			return bindingDescription;
		}

		/**
		 * @brief Get the attribute descriptions for per-instance data.
		 *
		 * The instance model matrix is exposed as four consecutive vec4
		 * attributes (locations 3 to 6), one per column of the matrix.
		 *
		 * @return std::array<VkVertexInputAttributeDescription, 4> The
		 *         instance attribute descriptions.
		 */
		static std::array<VkVertexInputAttributeDescription, 4>
			getInstanceAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 4>
				attributeDescriptions {};

			for (std::size_t i = 0; i < 4; ++i) {
				attributeDescriptions[i].binding  = 1;
				attributeDescriptions[i].location = static_cast<uint32_t>(3 + i);
				attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
				attributeDescriptions[i].offset =
					static_cast<uint32_t>(i * sizeof(glm::vec4));
			}

			return attributeDescriptions;
		}

		/**
		 * @brief Equality operator for GPUVertex.
		 *
		 * This operator checks if two GPUVertex instances are equal by
		 * comparing their position, color, and texture coordinates. It returns
		 * true if all attributes are equal, and false otherwise.
		 *
		 * @param other The other GPUVertex instance to compare with.
		 *
		 * @return true if the two GPUVertex instances are equal, false
		 * otherwise.
		 */
		bool operator==(const GPUVertex &other) const
		{
			return pos == other.pos && color == other.color
				&& texCoord == other.texCoord;
		}

		/**
		 * @brief Create a GPUVertex from a utility::graphic::Vertex object.
		 *
		 * This static function takes a utility::graphic::Vertex object as
		 * input and converts it into a GPUVertex object. It extracts the
		 * position, color, and texture coordinates from the Vertex object and
		 * maps them to the corresponding attributes of the GPUVertex. The color
		 * is converted from a Color32Bit format (with components in the range
		 * [0, 255]) to a normalized RGB format (with components in the range
		 * [0.0, 1.0]) suitable for use in shaders.
		 *
		 * @param vertex The utility::graphic::VertexF object to convert.
		 * @return GPUVertex The resulting GPUVertex object with the converted
		 * attributes.
		 */
		static GPUVertex
			createFromVertex(const utility::graphic::VertexF &vertex)
		{
			auto color32Bit = vertex.getColor();
			auto red		= color32Bit.getRed() / 255.0f;
			auto green		= color32Bit.getGreen() / 255.0f;
			auto blue		= color32Bit.getBlue() / 255.0f;

			return GPUVertex { glm::vec3(vertex.getPosition().x,
										 vertex.getPosition().y,
										 vertex.getPosition().z),
							   glm::vec3(red, green, blue),
							   glm::vec2(vertex.getTextureCoordinates().x,
										 vertex.getTextureCoordinates().y) };
		}
	};
}	 // namespace evan

#endif /* !GPUVERTEX_HPP_ */
