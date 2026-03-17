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

		Material(const DeviceContext &deviceContext, const Renderer &renderer,
				 const std::string &texturePath);
		~Material();

		void destroy(VkDevice device);

		void createImage(const ADeviceBackend &deviceBackend,
						 const std::string &texturePath,
						 VkCommandPool commandPool, VkQueue graphicsQueue);

		void createImageView(const ADeviceBackend &deviceBackend);

		void createSampler(const ADeviceBackend &deviceBackend,
						   VkSamplerCreateInfo samplerInfo);

		void createDescriptorSets(VkDevice logicalDevice,
								  VkDescriptorSetLayout descriptorSetLayout,
								  VkDescriptorPool descriptorPool,
								  const std::vector<VkBuffer> &uniformBuffers);

		std::vector<VkDescriptorSet> getDescriptorSets() const;

		protected:
		VkImage _image;
		VkDeviceMemory _memory;
		VkImageView _view;
		VkSampler _sampler;
		std::vector<VkDescriptorSet> _descriptorSets;
		uint32_t _mipLevel;

		void generateMipmaps(const GenerateMipmapsProperties &properties,
							 const ADeviceBackend &deviceBackend);

		VkSamplerCreateInfo getDefaultSamplerInfo(
			const VkPhysicalDeviceProperties &properties);

		private:
	};
}	 // namespace evan
