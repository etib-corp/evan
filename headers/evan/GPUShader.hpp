/*
** ETIB PROJECT, 2025
** evan
** File description:
** GPUShader
*/

#pragma once

#include <utility/graphic/shader.hpp>

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include "evan/EvanPlatform.hpp"

namespace evan
{
	/**
	 * @class GPUShader
	 * @brief The GPUShader class represents a shader used in rendering.
	 *
	 * The GPUShader class encapsulates the vertex and fragment shader modules,
	 * as well as the SPIR-V code for both shaders. It provides methods to
	 * retrieve the shader modules and to destroy the shader modules when they
	 * are no longer needed.
	 */
	class GPUShader:
		protected utility::logging::Loggable<GPUShader,
											 utility::logging::DefaultLogger>
	{
		public:
		/**
		 * @brief Constructs a GPUShader object with the given vertex and
		 * fragment shader code.
		 *
		 * @param device The Vulkan device used to create the shader modules.
		 * @param shader A reference to the utility::graphic::Shader object
		 * containing the shader data.
		 */
		GPUShader(VkDevice device, const utility::graphic::Shader &shader);

		~GPUShader();

		/**
		 * @brief Destroys the shader modules associated with this GPUShader
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

		/**
		 * Vulkan logical device used to create and manage shader modules
		 */
		VkDevice _logicalDevice;
	};
}	 // namespace evan
