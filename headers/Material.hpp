/*
** ETIB PROJECT, 2026
** evan
** File description:
** Material
*/

#pragma once

#include "EvanPlatform.hpp"

#include "ADeviceBackend.hpp"
#include "DeviceContext.hpp"

namespace evan
{
	class Renderer;

	/**
	 * @class Material
	 *
	 * @brief Represents a material in the rendering engine, encapsulating
	 * texture data, image views, samplers, and descriptor sets for use in
	 * Vulkan rendering operations.
	 *
	 * The Material class is responsible for managing the Vulkan resources
	 * associated with a material, including creating images from texture files,
	 * generating mipmaps, creating image views and samplers, and setting up
	 * descriptor sets for shader access. It provides methods for initializing
	 * these resources based on a given texture path and the device context and
	 * renderer instances. The class also includes functionality for cleaning up
	 * Vulkan resources when they are no longer needed.
	 */
	class Material
	{
		public:
		/**
		 * @struct GenerateMipmapsProperties
		 * @brief Holds properties required for generating mipmaps for a Vulkan
		 * image.
		 *
		 * This structure encapsulates all necessary Vulkan objects and image
		 * properties needed to perform mipmap generation operations.
		 *
		 */
		struct GenerateMipmapsProperties {
			/*
			 * @brief The Vulkan command pool used to allocate command buffers
			 * for mipmap generation.
			 */
			VkCommandPool _commandPool;
			/*
			 * @brief The Vulkan graphics queue used to submit the command
			 * buffer for execution.
			 */
			VkQueue _graphicsQueue;
			/*
			 * @brief The Vulkan image for which mipmaps are to be generated.
			 */
			VkImage _image;
			/*
			 * @brief The format of the image for which mipmaps are to be
			 * generated.
			 */
			VkFormat _imageFormat;
			/*
			 * @brief The width of the image in pixels.
			 */
			uint32_t _texWidth;
			/*
			 * @brief The height of the image in pixels.
			 */
			uint32_t _texHeight;
			/*
			 * @brief The number of mipmap levels to generate.
			 */
			uint32_t _mipLevels;
		};

		/**
		 * @brief Constructs a Material instance using the provided device
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
		 * @param texturePath A string representing the file path to the texture
		 * image that will be loaded and used for the material. The constructor
		 * will create a Vulkan image based on this texture file and generate
		 * mipmaps, image views, samplers, and descriptor sets accordingly.
		 *
		 * @note The constructor performs several Vulkan operations, including:
		 * - Creating a Vulkan image from the specified texture file.
		 * - Generating mipmaps for the created image.
		 * - Creating an image view for the Vulkan image.
		 * - Creating a sampler for the Vulkan image.
		 * - Setting up descriptor sets for shader access to the material's
		 * texture data.
		 */
		Material(const DeviceContext &deviceContext, const Renderer &renderer,
				 const std::string &texturePath);

		~Material();

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

		protected:
		/**
		 * @brief Creates a Vulkan image from the specified texture file and
		 * initializes it for use in the material.
		 *
		 * This method loads the texture image from the given file path, creates
		 * a Vulkan image resource, allocates memory for it, and transitions it
		 * to the appropriate layout for shader access. It also generates
		 * mipmaps for the image if necessary.
		 *
		 * @param deviceBackend A reference to the ADeviceBackend instance,
		 * which provides access to Vulkan device resources and configurations
		 * needed for creating the image and performing necessary operations.
		 * @param texturePath A string representing the file path to the texture
		 * image that will be loaded and used to create the Vulkan image for
		 * this material.
		 * @param commandPool The Vulkan command pool used to allocate command
		 * buffers for image operations such as layout transitions and mipmap
		 * generation.
		 * @param graphicsQueue The Vulkan graphics queue used to submit command
		 * buffers for execution during image operations.
		 *
		 * @note The method performs several Vulkan operations, including:
		 * - Loading the texture image from the specified file path.
		 * - Creating a Vulkan image resource based on the loaded texture data.
		 * - Allocating memory for the Vulkan image and binding it.
		 * - Transitioning the image layout to the appropriate state for shader
		 * access.
		 * - Generating mipmaps for the image if necessary.
		 */
		void createImage(const ADeviceBackend &deviceBackend,
						 const std::string &texturePath,
						 VkCommandPool commandPool, VkQueue graphicsQueue);

		/**
		 * @brief Creates a Vulkan image view for the material's Vulkan image.
		 *
		 * This method creates an image view for the Vulkan image associated
		 * with this material, allowing shaders to access the image data in a
		 * specific format and layout. The image view is essential for using the
		 * Vulkan image in rendering operations, as it defines how the image
		 * data is interpreted and accessed by the GPU.
		 *
		 * @param deviceBackend A reference to the ADeviceBackend instance,
		 * which provides access to Vulkan device resources and configurations
		 * needed for creating the image view.
		 *
		 * @note The method performs several Vulkan operations, including:
		 * - Creating an image view for the Vulkan image associated with this
		 * material.
		 * - Configuring the image view with the appropriate format, aspect
		 * flags, and mip levels based on the material's Vulkan image
		 * properties.
		 * - Storing the created image view for use in shader access and
		 * rendering operations.
		 */
		void createImageView(const ADeviceBackend &deviceBackend);

		/**
		 * @brief Creates a Vulkan sampler for the material, allowing shaders to
		 * sample the material's texture data during rendering operations.
		 *
		 * This method creates a Vulkan sampler resource that defines how the
		 * material's texture data is sampled in shader programs. The sampler
		 * configuration includes filtering modes, addressing modes, and other
		 * parameters that control how the texture is accessed and filtered
		 * during rendering.
		 *
		 * @param deviceBackend A reference to the ADeviceBackend instance,
		 * which provides access to Vulkan device resources and configurations
		 * needed for creating the sampler.
		 * @param samplerInfo A VkSamplerCreateInfo structure containing the
		 * configuration parameters for the sampler, such as filtering modes,
		 * addressing modes, and other relevant settings for texture sampling.
		 *
		 * @note The method performs several Vulkan operations, including:
		 * - Creating a Vulkan sampler resource based on the provided
		 * configuration parameters.
		 * - Storing the created sampler for use in shader access and rendering
		 * operations.
		 */
		void createSampler(const ADeviceBackend &deviceBackend,
						   VkSamplerCreateInfo samplerInfo);

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
		 * @brief Generates mipmaps for the material's Vulkan image using the
		 * provided properties and device backend.
		 *
		 * This method performs mipmap generation for the Vulkan image
		 * associated with this material, utilizing the provided properties and
		 * device backend to access necessary Vulkan resources and
		 * configurations. The generated mipmaps allow for improved texture
		 * sampling quality at various distances and levels of detail during
		 * rendering operations.
		 *
		 * @param properties A GenerateMipmapsProperties structure containing
		 * the necessary Vulkan objects and image properties required for mipmap
		 * generation, such as command pool, graphics queue, image, format,
		 * dimensions, and mip levels.
		 * @param deviceBackend A reference to the ADeviceBackend instance,
		 * which provides access to Vulkan device resources and configurations
		 * needed for performing mipmap generation operations.
		 *
		 * @note The method performs several Vulkan operations, including:
		 * - Allocating command buffers from the provided command pool for
		 * recording mipmap generation commands.
		 * - Recording commands to transition the image layout, blit the image
		 * to generate mipmaps, and transition the image layout back to the
		 * appropriate state for shader access.
		 * - Submitting the recorded command buffer to the provided graphics
		 * queue for execution, ensuring that the mipmap generation operations
		 * are completed before the material's Vulkan image is used in rendering
		 * operations.
		 */
		void generateMipmaps(const GenerateMipmapsProperties &properties,
							 const ADeviceBackend &deviceBackend);

		/**
		 * @brief Retrieves a default VkSamplerCreateInfo structure configured
		 * based on the provided physical device properties.
		 *
		 * This method generates a default VkSamplerCreateInfo structure that
		 * can be used to create a Vulkan sampler for the material. The
		 * configuration of the sampler is based on the properties of the
		 * physical device, such as supported filtering modes and addressing
		 * modes, to ensure compatibility and optimal performance when sampling
		 * the material's texture data in shader programs.
		 *
		 * @param properties A reference to the VkPhysicalDeviceProperties
		 * structure containing the properties of the physical device. This
		 * information is used to configure the default sampler settings
		 * appropriately for the device's capabilities.
		 *
		 * @return A VkSamplerCreateInfo structure containing the default
		 * configuration for creating a Vulkan sampler for this material, based
		 * on the provided physical device properties. This structure can be
		 * used directly in the createSampler method to create a sampler with
		 * default settings.
		 */
		VkSamplerCreateInfo
			getDefaultSamplerInfo(const VkPhysicalDeviceProperties &properties);

		/**
		 * Vulkan image resource associated with the material, representing the
		 * texture data loaded from the specified file path.
		 */
		VkImage _image;

		/**
		 * Vulkan device memory resource associated with the material's image.
		 */
		VkDeviceMemory _memory;

		/**
		 * Vulkan image view associated with the material's image, allowing
		 * shaders to access the image data in a specific format and layout.
		 */
		VkImageView _view;

		/**
		 * Vulkan sampler associated with the material, defining how the
		 * material's texture data is sampled in shader programs.
		 */
		VkSampler _sampler;

		/**
		 * Vector of Vulkan descriptor sets associated with the material.
		 */
		std::vector<VkDescriptorSet> _descriptorSets;

		/**
		 * The number of mipmap levels for the material's texture.
		 */
		uint32_t _mipLevel;
	};
}	 // namespace evan
