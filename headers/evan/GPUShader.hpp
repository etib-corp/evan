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

#include <cstdint>
#include <vector>

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
		 * @note This method is idempotent: after a module is destroyed its
		 * handle is reset to VK_NULL_HANDLE, so calling destroy() multiple
		 * times is safe. It is also invoked automatically by the destructor.
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

		/**
		 * @brief Tells whether a vertex shader declares the instance matrix.
		 *
		 * The renderer binds one model matrix per drawn instance as four
		 * `vec4` Input variables at locations 3 to 6 (see
		 * `GPUVertex::getInstanceAttributeDescriptions`). Only a vertex shader
		 * that declares all four of them places every instance at its own
		 * transform; merging identical meshes into one instanced draw is
		 * correct only for such a shader, so the renderer asks this before
		 * batching a run of draws.
		 *
		 * The check reads the input interface of @p vertexCode directly, so it
		 * reflects what the SPIR-V module exposes rather than what the
		 * application intended: a shader optimized down to locations 0-2 (the
		 * `default.vert` / `text.vert` shape) is reported as not instanced.
		 *
		 * @param vertexCode SPIR-V words of the vertex shader module.
		 * @return True when every instance matrix location is declared as an
		 * Input variable.
		 */
		[[nodiscard]] static bool
			declaresInstanceInputs(const std::vector<uint32_t> &vertexCode);

		/**
		 * @brief Tells whether this vertex shader declares the instance
		 * matrix.
		 *
		 * The answer is resolved once, when the module is created, so the
		 * renderer can ask it for every draw of the frame without walking the
		 * SPIR-V input interface again.
		 *
		 * @return True when the vertex shader consumes the per-instance model
		 * matrix, false otherwise.
		 */
		[[nodiscard]] bool declaresInstanceInputs() const;

		protected:
		/**
		 * @brief Releases the Vulkan shader modules owned by this shader.
		 *
		 * Idempotent: modules are null-checked and reset after destruction.
		 */
		void cleanup();

		/**
		 * Vulkan shader module for the vertex shader
		 */
		VkShaderModule _vertexShaderModule = VK_NULL_HANDLE;

		/**
		 * Vulkan shader module for the fragment shader
		 */
		VkShaderModule _fragmentShaderModule = VK_NULL_HANDLE;

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
		VkDevice _logicalDevice = VK_NULL_HANDLE;

		/**
		 * Whether the vertex shader declares the per-instance model matrix
		 * (Input locations 3 to 6).
		 *
		 * Resolved once from the SPIR-V module; see declaresInstanceInputs.
		 */
		bool _declaresInstanceInputs = false;
	};
}	 // namespace evan
