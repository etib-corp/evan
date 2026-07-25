/*
** ETIB PROJECT, 2026
** evan
** File description:
** GPUMaterial
*/

#pragma once

#include "evan/EvanPlatform.hpp"

#include <utility/graphic/material.hpp>

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include "evan/ADeviceBackend.hpp"
#include "evan/DeviceContext.hpp"

#include "GPUTexture.hpp"

namespace evan
{
	class Renderer;

	/**
	 * @class GPUMaterial
	 *
	 * @brief Represents a material in the rendering engine, encapsulating
	 * texture data, image views, samplers, and descriptor sets for use in
	 * Vulkan rendering operations.
	 *
	 * The GPUMaterial class is responsible for managing the Vulkan resources
	 * associated with a material, including creating images from texture files,
	 * generating mipmaps, creating image views and samplers, and setting up
	 * descriptor sets for shader access. It provides methods for initializing
	 * these resources based on a given texture path and the device context and
	 * renderer instances. The class also includes functionality for cleaning up
	 * Vulkan resources when they are no longer needed.
	 */
	class GPUMaterial:
		protected utility::logging::Loggable<GPUMaterial,
											 utility::logging::DefaultLogger>
	{
		public:
		/**
		 * @brief Constructs a GPUMaterial instance using the provided device
		 * context, renderer, and texture path.
		 *
		 * This constructor initializes the material by creating a Vulkan image
		 * from the specified texture file, generating mipmaps for the image,
		 * creating an image view and sampler, and setting up descriptor sets
		 * for shader access. It utilizes the provided device context and
		 * renderer to access necessary Vulkan resources and configurations for
		 * these operations.
		 *
		 * @param deviceContext A reference to the DeviceContext instance, which
		 * provides access to Vulkan device resources and configurations needed
		 * for creating images, image views, samplers, and descriptor sets.
		 * @param renderer A reference to the Renderer instance, which may be
		 * used to access rendering configurations or resources needed for
		 * material initialization.
		 * @param material A reference to the utility::graphic::Material
		 * instance, which contains information about the material's shader and
		 * textures, used to initialize the Vulkan resources for this material.
		 * @param shaderID The unique ID of the shader associated with this
		 * material, used to identify which shader program should be used when
		 * rendering objects that utilize this material.
		 *
		 * @note The constructor performs several Vulkan operations, including:
		 * - Creating a Vulkan image from the specified texture file.
		 * - Generating mipmaps for the created image.
		 * - Creating an image view for the Vulkan image.
		 * - Creating a sampler for the Vulkan image.
		 * - Setting up descriptor sets for shader access to the material's
		 * texture data.
		 */
		GPUMaterial(std::shared_ptr<DeviceContext> deviceContext,
					const Renderer &renderer,
					const utility::graphic::Material &material,
					uint32_t shaderID);

		~GPUMaterial();

		/**
		 * @brief Destroys the Vulkan resources associated with the material.
		 *
		 * This method cleans up all Vulkan resources allocated for the
		 * material, including the Vulkan image, image view, sampler, and
		 * descriptor sets. It should be called when the material is no longer
		 * needed to free up GPU resources and prevent memory leaks.
		 *
		 * @param device The Vulkan logical device used to destroy the
		 * associated resources. This device should be the same one used to
		 * create the resources in the first place.
		 *
		 * @note The method performs several Vulkan cleanup operations,
		 * including:
		 * - Destroying the Vulkan image associated with the material.
		 * - Freeing the memory allocated for the Vulkan image.
		 * - Destroying the image view associated with the Vulkan image.
		 * - Destroying the sampler associated with the material.
		 * - Cleaning up any descriptor sets associated with the material.
		 */
		void destroy(VkDevice device);

		/**
		 * @brief Updates the GPUMaterial with new texture data and resources.
		 *
		 * This method updates the GPUMaterial by reinitializing its Vulkan
		 * resources based on the provided material's texture data.
		 * It creates new Vulkan images, image views, samplers,
		 * and descriptor sets as needed to reflect any changes in the
		 * material's properties or associated textures.
		 *
		 * @param deviceContext A shared pointer to the DeviceContext instance,
		 * which provides access to Vulkan device resources and configurations
		 * needed for updating images, image views, samplers, and descriptor
		 * sets.
		 * @param renderer A reference to the Renderer instance, which may be
		 * used to access rendering configurations or resources needed for
		 * material updates.
		 * @param material A reference to the utility::graphic::Material
		 * instance, which contains updated information about the material's
		 * shader and textures, used to reinitialize the Vulkan resources for
		 * this material.
		 * @param shaderID The unique ID of the shader associated with this
		 * material, used to identify which shader program should be used when
		 * rendering objects that utilize this material.
		 */
		void update(std::shared_ptr<DeviceContext> deviceContext,
					const Renderer &renderer,
					const utility::graphic::Material &material,
					uint32_t shaderID);

		/**
		 * @brief Retrieves the Vulkan descriptor sets associated with this
		 * material.
		 *
		 * This method returns a vector of Vulkan descriptor sets that have been
		 * created for this material. These descriptor sets can be used in
		 * shader programs to access the material's texture data and other
		 * associated resources during rendering operations.
		 *
		 * @return A vector of VkDescriptorSet objects representing the
		 * descriptor sets associated with this material. These descriptor sets
		 * can be bound to shader stages to provide access to the material's
		 * resources during rendering.
		 */
		std::vector<VkDescriptorSet> getDescriptorSets() const;

		/**
		 * @brief Retrieves the unique shader ID associated with this material.
		 *
		 * This method returns the unique ID of the shader that is associated
		 * with this material. The shader ID is used to identify which shader
		 * program should be used when rendering objects that utilize this
		 * material, allowing for proper rendering of the material's visual
		 * properties based on the shader's functionality and characteristics.
		 *
		 * @return uint32_t The unique shader ID associated with this material,
		 * used to identify the shader program for rendering objects that use
		 * this material.
		 */
		uint32_t getShaderID() const;

		/**
		 * @brief Retrieves the version number of the uploaded material.
		 *
		 * This method returns the version number of the material that was
		 * uploaded to the GPU. This is useful for tracking changes and updates
		 * to the material's properties and resources.
		 *
		 * @return uint32_t The version number of the uploaded material.
		 */
		uint32_t getUploadedVersion() const;

		protected:
		/**
		 * @brief Creates Vulkan descriptor sets for the material, allowing
		 * shaders to access the material's texture data and associated
		 * resources during rendering operations.
		 *
		 * This method sets up descriptor sets for the material, which can be
		 * bound to shader stages to provide access to the material's Vulkan
		 * image, image view, sampler, and other associated resources. The
		 * descriptor sets are configured based on the provided descriptor set
		 * layout and uniform buffers.
		 *
		 * @param logicalDevice The Vulkan logical device used to create the
		 * descriptor sets. This device should be the same one used to create
		 * the material's Vulkan resources.
		 * @param descriptorSetLayout The Vulkan descriptor set layout that
		 * defines the structure of the descriptor sets to be created for this
		 * material. The layout should match the expected bindings in the shader
		 * programs that will access the material's resources.
		 * @param descriptorPool The Vulkan descriptor pool from which the
		 * descriptor sets will be allocated. This pool should have sufficient
		 * capacity to allocate the required number of descriptor sets for this
		 * material.
		 * @param uniformBuffers A vector of Vulkan buffer handles representing
		 * uniform buffers that may be used in the descriptor sets for shader
		 * access to uniform data associated with the material.
		 *
		 * @note The method performs several Vulkan operations, including:
		 * - Allocating descriptor sets from the specified descriptor pool based
		 * on the provided descriptor set layout.
		 * - Configuring the allocated descriptor sets with the material's
		 * Vulkan image, image view, sampler, and any provided uniform buffers
		 * according to the expected bindings in the shader programs.
		 * - Storing the created descriptor sets for use in shader access and
		 * rendering operations.
		 */
		void createDescriptorSets(VkDevice logicalDevice,
								  VkDescriptorSetLayout descriptorSetLayout,
								  VkDescriptorPool descriptorPool,
								  const std::vector<VkBuffer> &uniformBuffers);

		/**
		 * Textures associated with the material, categorized by their intended
		 * use in the rendering pipeline.
		 */
		std::vector<std::shared_ptr<GPUTexture>> _textures;

		/**
		 * The unique ID associated with the shader used by this material.
		 */
		uint32_t _shaderID;

		/**
		 * Vector of Vulkan descriptor sets associated with the material.
		 */
		std::vector<VkDescriptorSet> _descriptorSets;

		/**
		 * The version number of the material, used for tracking changes and
		 * updates to the material's properties and resources.
		 */
		uint32_t _uploadedVersion = 0;

		private:
		uint32_t getBinding(GPUTexture::TextureType type);
	};
}	 // namespace evan
