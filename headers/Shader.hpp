/*
** ETIB PROJECT, 2025
** evan
** File description:
** Shader
*/

#pragma once

#include "EvanPlatform.hpp"

namespace evan
{
	/**
	 * @class Shader
	 * @brief The Shader class represents a shader used in rendering.
	 *
	 * The Shader class encapsulates the vertex and fragment shader modules, as
	 * well as the SPIR-V code for both shaders. It provides methods to retrieve
	 * the shader modules and to destroy the shader modules when they are no
	 * longer needed.
	 */
	class Shader
	{
		public:
		/**
		 * @brief Constructs a Shader object with the given vertex and fragment
		 * shader code.
		 *
		 * @param vertexCode A vector containing the SPIR-V code for the vertex
		 * shader.
		 * @param fragmentCode A vector containing the SPIR-V code for the
		 * fragment shader.
		 * @param device The Vulkan device used to create the shader modules.
		 */
		Shader(const std::vector<uint32_t> &vertexCode,
			   const std::vector<uint32_t> &fragmentCode, VkDevice device);

		~Shader();

		/**
		 * @brief Destroys the shader modules associated with this Shader
		 * object.
		 *
		 * @note This method should be called when the shader modules are no
		 * longer needed to free up resources.
		 */
		void destroy();

		/**
		 * @brief Retrieves the vertex shader module.
		 *
		 * @return The Vulkan shader module for the vertex shader.
		 */
		VkShaderModule getVertexShaderModule() const;

		/**
		 * @brief Retrieves the fragment shader module.
		 *
		 * @return The Vulkan shader module for the fragment shader.
		 */
		VkShaderModule getFragmentShaderModule() const;

		protected:
		/**
		 * Vulkan shader module for the vertex shader
		 */
		VkShaderModule _vertexShaderModule;

		/**
		 * Vulkan shader module for the fragment shader
		 */
		VkShaderModule _fragmentShaderModule;

		/**
		 * SPIR-V code for the vertex shader
		 */
		std::vector<uint32_t> _vertexCode;

		/**
		 * SPIR-V code for the fragment shader
		 */
		std::vector<uint32_t> _fragmentCode;

		VkDevice _logicalDevice;
	};
}	 // namespace evan
