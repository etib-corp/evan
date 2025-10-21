/*
** ETIB PROJECT, 2025
** evan
** File description:
** Vertex
*/

#ifndef VERTEX_HPP_
#define VERTEX_HPP_

#include "evan.hpp"

namespace evan {
/**
 * @brief Vertex structure.
 *
 * This structure represents a vertex in the 3D space. It contains the
 * position, color, and texture coordinates of the vertex. It is used to
 * define the vertex data for rendering.
 *
 */
struct Vertex {
  glm::vec3 pos;      // Position of the vertex
  glm::vec3 color;    // Color of the vertex    (TODO: change to vec4 for alpha)
  glm::vec2 texCoord; // Texture coordinates of the vertex

  /*
   * @brief Get the binding description for the vertex.
   *
   * This function returns the binding description for the vertex,
   * which specifies how the vertex data is laid out in memory.
   *
   * @return VkVertexInputBindingDescription The binding description
   *         for the vertex.
   *
   */
  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  /*
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
  getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
  }

  /*
   * @brief Equality operator for the Vertex structure.
   *
   * This operator compares two Vertex objects for equality based on
   * their position, color, and texture coordinates.
   *
   * @param other The other Vertex object to compare with.
   * @return true if the two Vertex objects are equal, false otherwise.
   *
   */
  bool operator==(const Vertex &other) const {
    return pos == other.pos && color == other.color &&
           texCoord == other.texCoord;
  }
};
} // namespace evan

#endif /* !VERTEX_HPP_ */
