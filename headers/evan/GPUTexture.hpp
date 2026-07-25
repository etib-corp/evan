/*
** ETIB PROJECT, 2026
** xider
** File description:
** GPUTexture
*/

#pragma once

#include <utility/graphic/texture.hpp>

#include "utility/logging/logger.hpp"
#include "utility/logging/default_logger.hpp"

#include "EvanPlatform.hpp"

#include "DeviceContext.hpp"

#include <cmath>
#include <math.h>
#include <algorithm>

namespace evan
{
	/**
	 * @brief GPUTexture class.
	 *
	 * This class represents a texture that can be used in the GPU for
	 * rendering. It encapsulates the Vulkan image, image view, and sampler
	 * associated with the texture, as well as the type of texture (e.g.,
	 * albedo, normal, roughness) for categorization in the rendering pipeline.
	 * The GPUTexture class provides functionality for creating and managing
	 * Vulkan resources related to textures, including image creation, memory
	 * allocation, image view setup, sampler creation, and mipmap generation. It
	 * also includes a method for destroying the Vulkan resources when they are
	 * no longer needed.
	 *
	 * @note The GPUTexture class is designed to work with the Vulkan API and is
	 * intended to be used in conjunction with a device backend that provides
	 * access to the Vulkan device and related resources. It is important to
	 * ensure that the GPUTexture is properly managed and destroyed to prevent
	 * resource leaks in the Vulkan application.
	 */
	class GPUTexture:
		protected utility::logging::Loggable<GPUTexture,
											 utility::logging::DefaultLogger>
	{
		public:
		/**
		 * @brief Enum class representing different types of textures that can
		 * be associated with a material, such as albedo, normal, and roughness
		 * textures.
		 *
		 * This enum class is used to categorize textures based on their
		 * intended use in the rendering pipeline. Each texture type corresponds
		 * to a specific role in defining the visual properties of a material,
		 * allowing for organized management of texture resources and their
		 * bindings in shader programs.
		 */
		enum class TextureType {
			Albedo,		  ///< Represents the base color texture of a material,
						  ///< defining its diffuse color.
			Normal,		  ///< Represents the normal map texture, which encodes
						  ///< surface normals for detailed lighting effects.
			Roughness,	  ///< Represents the roughness texture, which defines
						  ///< the surface roughness for specular reflections.
			FontAtlas	  ///< Represents the font atlas texture, which contains
						  ///< pre-rendered glyphs for text rendering.
		};

		/**
		 * @struct GenerateMipmapsProperties
		 * @brief Holds properties required for generating mipmaps for a Vulkan
		 * image.
		 *
		 * This structure encapsulates all necessary Vulkan objects and image
		 * properties needed to perform mipmap generation operations.
		 *
		 * @param _commandPool The Vulkan command pool used to allocate command
		 * buffers for mipmap generation.
		 * @param _graphicsQueue The Vulkan graphics queue used to submit
		 * command buffers for execution during mipmap generation.
		 * @param _image The Vulkan image for which mipmaps are to be generated.
		 * @param _imageFormat The format of the Vulkan image for which mipmaps
		 * are to be generated.
		 * @param _texWidth The width of the Vulkan image in pixels.
		 * @param _texHeight The height of the Vulkan image in pixels.
		 * @param _mipLevels The number of mipmap levels to generate for the
		 * Vulkan image.
		 */
		struct GenerateMipmapsProperties {
			VkCommandPool
				_commandPool;	 ///< The Vulkan command pool used to allocate
								 ///< command buffers for mipmap generation.

			VkQueue _graphicsQueue;	   ///< The Vulkan graphics queue used to
									   ///< submit command buffers for execution
									   ///< during mipmap generation.

			VkImage _image;	   ///< The Vulkan image for which mipmaps are to be
							   ///< generated.

			VkFormat _imageFormat;	  ///< The format of the Vulkan image for
									  ///< which mipmaps are to be generated.

			uint32_t _texWidth;	   ///< The width of the Vulkan image in pixels.

			uint32_t
				_texHeight;	   ///< The height of the Vulkan image in pixels.

			uint32_t _mipLevels;	///< The number of mipmap levels to generate
									///< for the Vulkan image.
		};

		/**
		 * @brief Constructs a GPUTexture object by creating a Vulkan image,
		 * image view, and sampler based on the provided texture data and type.
		 *
		 * This constructor initializes a GPUTexture instance by creating a
		 * Vulkan image using the provided texture data, setting up the
		 * corresponding image view and sampler for use in shader programs. The
		 * constructor takes into account the type of texture being created
		 * (e.g., albedo, normal, roughness) to determine the appropriate format
		 * and usage flags for the Vulkan image, ensuring that the texture is
		 * correctly configured for its intended role in the rendering pipeline.
		 *
		 * @param deviceContext A reference to the DeviceContext instance,
		 * which provides access to the Vulkan device and related resources
		 * needed for creating the image, image view, and sampler.
		 * @param texture A reference to a utility::graphic::Texture object
		 * containing the pixel data and dimensions of the texture to be
		 * created.
		 * @param type The type of texture being created (e.g., albedo, normal,
		 * roughness), which determines the format and usage flags for the
		 * Vulkan image.
		 *
		 * @note The constructor performs several Vulkan operations, including:
		 * - Creating a Vulkan image with the appropriate format and usage flags
		 * based on the texture type.
		 * - Allocating memory for the Vulkan image and binding it to the image
		 * object.
		 * - Creating an image view for the Vulkan image to allow shader access.
		 * - Creating a sampler for the Vulkan image to define how the texture
		 * is sampled in shaders.
		 * - Generating mipmaps for the Vulkan image if necessary, based on the
		 * texture dimensions
		 */
		GPUTexture(const DeviceContext &deviceContext,
				   const utility::graphic::Texture &texture,
				   TextureType type = TextureType::Albedo);

		/**
		 * @brief Destructs the GPUTexture object by cleaning up Vulkan
		 * resources such as the image, image view, and sampler.
		 */
		~GPUTexture();

		/**
		 * @brief Destroys the Vulkan resources associated with this GPUTexture,
		 * including the image, image view, and sampler.
		 *
		 * @param device The Vulkan logical device used to destroy the
		 * associated resources. This parameter is required to ensure that the
		 * correct device context is used for resource cleanup, preventing
		 * potential issues with resource management and ensuring proper
		 * deallocation of Vulkan objects.
		 *
		 * @note This method should be called when the GPUTexture is no longer
		 * needed to free up Vulkan resources and prevent memory leaks. It is
		 * important to ensure that the device parameter corresponds to the same
		 * logical device used to create the Vulkan resources associated with
		 * this GPUTexture. Failure to call this method or providing an
		 * incorrect device context may result in resource leaks or undefined
		 * behavior in the Vulkan application.
		 */
		void destroy(VkDevice device);

		/**
		 * @brief The Vulkan image view associated with this GPUTexture.
		 */
		VkImageView view;

		/**
		 * @brief The Vulkan sampler associated with this GPUTexture,
		 * which defines how the texture is sampled in shader programs.
		 */
		VkSampler sampler;

		/**
		 * @brief The type of texture represented by this GPUTexture instance,
		 * such as albedo, normal, or roughness. This property is used to
		 * categorize the texture based on its intended use in the rendering
		 * pipeline, allowing for organized management of texture resources and
		 * their bindings in shader programs.
		 */
		TextureType type;

		protected:
		/**
		 * @brief Creates a Vulkan image based on the provided texture data and
		 * type, and allocates memory for it.
		 *
		 * This method is responsible for creating a Vulkan image object using
		 * the pixel data and dimensions from the provided
		 * utility::graphic::Texture, as well as determining the appropriate
		 * format and usage flags based on the specified TextureType. It also
		 * allocates memory for the Vulkan image and binds it to the image
		 * object, ensuring that the texture data is properly stored in GPU
		 * memory for use in rendering operations.
		 *
		 * @param deviceBackend A reference to the ADeviceBackend instance,
		 * which provides access to the Vulkan device and related resources
		 * needed for creating the image and allocating memory.
		 * @param texture A reference to a utility::graphic::Texture object
		 * containing the pixel data and dimensions of the texture to be
		 * created.
		 * @param commandPool The Vulkan command pool used to allocate command
		 * buffers for any necessary staging operations during image creation.
		 * @param graphicsQueue The Vulkan graphics queue used to submit command
		 * buffers for execution during image creation and any necessary staging
		 * operations.
		 */
		void createImage(const ADeviceBackend &deviceBackend,
						 const utility::graphic::Texture &texture,
						 VkCommandPool commandPool, VkQueue graphicsQueue);

		/**
		 * @brief Creates a Vulkan image view for the Vulkan image associated
		 * with this GPUTexture, allowing shader access to the texture data.
		 *
		 * This method sets up the image view for the Vulkan image, which is
		 * necessary for shaders to access the texture data correctly. The image
		 * view defines how the image data is interpreted and accessed in shader
		 * programs, including aspects such as format and subresource range.
		 * Properly creating the image view ensures that the texture can be
		 * sampled correctly in rendering operations.
		 *
		 * @param deviceBackend A reference to the ADeviceBackend instance,
		 * which provides access to the Vulkan device and related resources
		 * needed for creating the image view.
		 */
		void createImageView(const ADeviceBackend &deviceBackend);

		/**
		 * @brief Creates a Vulkan sampler for the Vulkan image associated with
		 * this GPUTexture, defining how the texture is sampled in shader
		 * programs.
		 *
		 * This method sets up the sampler for the Vulkan image, which specifies
		 * how the texture is sampled in shader programs, including filtering
		 * modes, addressing modes, and mipmapping behavior. Properly creating
		 * the sampler ensures that the texture is sampled correctly in
		 * rendering operations, allowing for effects such as smooth filtering
		 * and proper handling of texture coordinates outside the [0, 1] range.
		 *
		 * @param deviceBackend A reference to the ADeviceBackend instance,
		 * which provides access to the Vulkan device and related resources
		 * needed for creating the sampler.
		 * @param samplerInfo A VkSamplerCreateInfo structure containing the
		 * parameters for creating the Vulkan sampler, such as filtering modes,
		 * addressing modes, and mipmapping behavior. This information is used
		 * to
		 */
		void createSampler(const ADeviceBackend &deviceBackend,
						   VkSamplerCreateInfo samplerInfo);

		/**
		 * @brief The Vulkan image associated with this GPUTexture, which holds
		 * the texture data in GPU memory for use in rendering operations.
		 */
		VkImage _image;

		/**
		 * @brief The Vulkan device memory allocated for the image associated
		 * with this GPUTexture, which is used to store the texture data in GPU
		 * memory.
		 */
		VkDeviceMemory _memory;

		/**
		 * @brief The number of mipmap levels generated for the Vulkan image
		 * associated with this GPUTexture, which is used to determine the level
		 * of detail for texture sampling in shader programs. This property is
		 * important for optimizing rendering performance and visual quality by
		 * allowing shaders to sample from different mipmap levels based on the
		 * distance and angle of the textured surface relative to the camera.
		 */
		uint32_t _mipLevel;

		private:
		/**
		 * @brief Generates mipmaps for the Vulkan image associated with this
		 * GPUTexture based on the provided properties, using Vulkan command
		 * buffers and appropriate synchronization to ensure correct execution
		 * of the mipmap generation process.
		 *
		 * This method performs the necessary Vulkan operations to generate
		 * mipmaps for the image, including transitioning image layouts, copying
		 * data between mipmap levels, and ensuring proper synchronization to
		 * avoid issues such as race conditions or incorrect rendering results.
		 * The method takes into account the properties provided in the
		 * GenerateMipmapsProperties structure, which includes the command pool,
		 * graphics queue, image, format, dimensions, and number of mipmap
		 * levels to generate. Properly generating mipmaps is crucial for
		 * optimizing rendering performance and visual quality, as it allows
		 * shaders to sample from different mipmap levels based on the distance
		 * and angle of the textured surface relative to the camera, reducing
		 * aliasing and improving the appearance of textures when viewed at
		 * various distances and angles.
		 *
		 * @param properties A GenerateMipmapsProperties structure containing
		 * all necessary Vulkan objects and parameters needed to perform mipmap
		 * generation operations, such as the command pool, graphics queue,
		 * image, format, dimensions, and number of mipmap levels to generate.
		 * This information is used to execute the mipmap generation process
		 * correctly and efficiently.
		 * @param deviceBackend A reference to the ADeviceBackend instance,
		 * which provides access to the Vulkan device and related resources
		 * needed for executing the mipmap generation process, including command
		 * buffer allocation and
		 */
		void generateMipmaps(const GenerateMipmapsProperties &properties,
							 const ADeviceBackend &deviceBackend);

		/**
		 * @brief Creates a default VkSamplerCreateInfo structure based on the
		 * provided Vulkan physical device properties, which can be used to
		 * create a Vulkan sampler for this GPUTexture.
		 *
		 * This method generates a default VkSamplerCreateInfo structure that
		 * defines the parameters for creating a Vulkan sampler, such as
		 * filtering modes, addressing modes, and mipmapping behavior. The
		 * method takes into account the properties of the Vulkan physical
		 * device to ensure that the sampler is configured correctly for the
		 * capabilities of the device. This default sampler information can be
		 * used when creating the Vulkan sampler for this GPUTexture, providing
		 * a convenient way to set up a sampler with reasonable defaults based
		 * on the device's capabilities.
		 *
		 * @param properties A VkPhysicalDeviceProperties structure containing
		 * the properties of the Vulkan physical device, which can be used to
		 * determine appropriate parameters for the default sampler, such as
		 * maximum anisotropy levels or supported filtering modes. This
		 * information is used to create a VkSamplerCreateInfo structure that is
		 * compatible with the capabilities of the device.
		 *
		 * @return A VkSamplerCreateInfo structure containing the default
		 * parameters for creating a Vulkan sampler for this GPUTexture, based
		 * on the properties of the Vulkan physical device. This structure can
		 * be used
		 */
		VkSamplerCreateInfo
			getDefaultSamplerInfo(const VkPhysicalDeviceProperties &properties);
	};
}	 // namespace evan
