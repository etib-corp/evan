/*
** ETIB PROJECT, 2026
** evan
** File description:
** Material
*/

#include "Material.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Renderer.hpp"

evan::Material::Material(const DeviceContext &deviceContext,
						 const Renderer &renderer,
						 const std::string &texturePath)
{
	auto deviceBackend = deviceContext.getDeviceBackend();

	this->createImage(*deviceBackend, texturePath,
					  deviceContext.getCommandPool(),
					  deviceContext.getGraphicsQueue());
	this->createImageView(*deviceBackend);

	// Passing the default sampler info to createSampler, which will be filled
	// with the max anisotropy value.
	VkSamplerCreateInfo samplerInfo {};
	samplerInfo.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter				= VK_FILTER_LINEAR;
	samplerInfo.minFilter				= VK_FILTER_LINEAR;
	samplerInfo.addressModeU			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable		= VK_TRUE;
	samplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable			= VK_FALSE;
	samplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias				= 0.0f;
	samplerInfo.minLod					= 0.0f;
	samplerInfo.maxLod					= 0.0f;
	samplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod					= 0.0f;
	samplerInfo.maxLod					= static_cast<float>(_mipLevel);
	samplerInfo.mipLodBias				= 0.0f;

	this->createSampler(*deviceBackend, samplerInfo);
	this->createDescriptorSets(
		deviceBackend->_device, renderer.getDescriptorSetLayout(),
		renderer.getDescriptorPool(), renderer.getUniformBuffers());
}

evan::Material::~Material()
{
}

void evan::Material::createImage(const ADeviceBackend &deviceBackend,
								 const std::string &texturePath,
								 VkCommandPool commandPool,
								 VkQueue graphicsQueue)
{
	int texWidth	= 0;
	int texHeight	= 0;
	int texChannels = 0;

	auto file				= g_assetManager->get(texturePath);
	std::string fileContent = file->content();
	stbi_uc *pixels			= stbi_load_from_memory(
		reinterpret_cast<const stbi_uc *>(fileContent.c_str()),
		fileContent.size(), &texWidth, &texHeight, &texChannels,
		STBI_rgb_alpha);

	VkDeviceSize imageSize = texWidth * texHeight * 4;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	if (!pixels) {
		throw std::runtime_error("Failed to load texture image !");
	}

	_mipLevel = static_cast<uint32_t>(
					std::floor(std::log2(std::max(texWidth, texHeight))))
		+ 1;

	ADeviceBackend::CreateBufferProperties stagingBufferProperties = {
		._logicalDevice	 = deviceBackend._device,
		._physicalDevice = deviceBackend._physicalDevice,
		._size			 = imageSize,
		._usage			 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		._properties	 = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		._buffer	   = stagingBuffer,
		._bufferMemory = stagingBufferMemory
	};

	deviceBackend.createBuffer(stagingBufferProperties);

	void *data;
	vkMapMemory(deviceBackend._device, stagingBufferMemory, 0, imageSize, 0,
				&data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(deviceBackend._device, stagingBufferMemory);

	stbi_image_free(pixels);

	ADeviceBackend::CreateImageProperties imageProperties = {
		._logicalDevice	 = deviceBackend._device,
		._physicalDevice = deviceBackend._physicalDevice,
		._width			 = (uint32_t)texWidth,
		._height		 = (uint32_t)texHeight,
		._mipLevels		 = _mipLevel,
		._numSamples	 = VK_SAMPLE_COUNT_1_BIT,
		._format		 = VK_FORMAT_R8G8B8A8_SRGB,
		._tiling		 = VK_IMAGE_TILING_OPTIMAL,
		._usage			 = VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		._properties  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._image		  = _image,
		._imageMemory = _memory
	};
	deviceBackend.createImage(imageProperties);

	ADeviceBackend::TransitionImageLayoutProperties transitionProperties = {
		._logicalDevice = deviceBackend._device,
		._commandPool	= commandPool,
		._graphicsQueue = graphicsQueue,
		._image			= _image,
		._format		= VK_FORMAT_R8G8B8A8_SRGB,
		._oldLayout		= VK_IMAGE_LAYOUT_UNDEFINED,
		._newLayout		= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		._mipLevels		= _mipLevel
	};
	deviceBackend.transitionImageLayout(transitionProperties);

	ADeviceBackend::CopyBufferToImageProperties copyProperties = {
		._logicalDevice = deviceBackend._device,
		._commandPool	= commandPool,
		._graphicsQueue = graphicsQueue,
		._buffer		= stagingBuffer,
		._image			= _image,
		._width			= (uint32_t)texWidth,
		._height		= (uint32_t)texHeight
	};
	deviceBackend.copyBufferToImage(copyProperties);

	GenerateMipmapsProperties propertiesMipmap = {
		._commandPool	= commandPool,
		._graphicsQueue = graphicsQueue,
		._image			= _image,
		._imageFormat	= VK_FORMAT_R8G8B8A8_SRGB,
		._texWidth		= (uint32_t)texWidth,
		._texHeight		= (uint32_t)texHeight,
		._mipLevels		= _mipLevel
	};
	this->generateMipmaps(propertiesMipmap, deviceBackend);

	vkDestroyBuffer(deviceBackend._device, stagingBuffer, nullptr);
	vkFreeMemory(deviceBackend._device, stagingBufferMemory, nullptr);
}

void evan::Material::createImageView(const ADeviceBackend &deviceBackend)
{
	_view = deviceBackend.createImageView(_image, VK_FORMAT_R8G8B8A8_SRGB,
										  VK_IMAGE_ASPECT_COLOR_BIT, _mipLevel);
}

void evan::Material::createSampler(const ADeviceBackend &deviceBackend,
								   VkSamplerCreateInfo samplerInfo)
{
	VkPhysicalDeviceProperties properties {};
	vkGetPhysicalDeviceProperties(deviceBackend._physicalDevice, &properties);

	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	// Check if samplerInfo is "empty" by testing its sType field.
	// If sType is not set, it's likely uninitialized.
	if (samplerInfo.sType == 0) {
		samplerInfo = this->getDefaultSamplerInfo(properties);
	}

	if (vkCreateSampler(deviceBackend._device, &samplerInfo, nullptr, &_sampler)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture sampler");
	}
}

void evan::Material::createDescriptorSets(
	VkDevice logicalDevice, VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	const std::vector<VkBuffer> &uniformBuffers)
{
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
											   descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo {};
	allocInfo.sType			 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts		 = layouts.data();

	_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(logicalDevice, &allocInfo,
								 _descriptorSets.data())
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets !");
	}
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo bufferInfo {};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range  = sizeof(Frame::UniformBufferObject);

		VkDescriptorImageInfo imageInfo {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView	  = _view;
		imageInfo.sampler	  = _sampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites {};

		descriptorWrites[0].sType	   = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet	   = _descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo		= &bufferInfo;

		descriptorWrites[1].sType	   = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet	   = _descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType =
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo		= &imageInfo;

		vkUpdateDescriptorSets(logicalDevice,
							   static_cast<uint32_t>(descriptorWrites.size()),
							   descriptorWrites.data(), 0, nullptr);
	}
}

///////////////////////
// Protected methods //
///////////////////////

VkSamplerCreateInfo evan::Material::getDefaultSamplerInfo(
	const VkPhysicalDeviceProperties &properties)
{
	VkSamplerCreateInfo samplerInfo {};
	samplerInfo.sType			 = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter		 = VK_FILTER_LINEAR;
	samplerInfo.minFilter		 = VK_FILTER_LINEAR;
	samplerInfo.addressModeU	 = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV	 = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW	 = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy	 = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor		 = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable			= VK_FALSE;
	samplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias				= 0.0f;
	samplerInfo.minLod					= 0.0f;
	samplerInfo.maxLod					= 1;
	return samplerInfo;
}

void evan::Material::generateMipmaps(
	const GenerateMipmapsProperties &properties,
	const ADeviceBackend &deviceBackend)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(deviceBackend._physicalDevice,
										properties._imageFormat,
										&formatProperties);

	if (!(formatProperties.optimalTilingFeatures
		  & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error(
			"texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = deviceBackend.beginSingleTimeCommands(
		deviceBackend._device, properties._commandPool);

	VkImageMemoryBarrier barrier {};
	barrier.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image				= properties._image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount		= 1;
	barrier.subresourceRange.levelCount		= 1;

	int32_t mipWidth  = properties._texWidth;
	int32_t mipHeight = properties._texHeight;

	for (uint32_t i = 1; i < properties._mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout	  = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout	  = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
							 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
							 nullptr, 1, &barrier);

		VkImageBlit blit {};
		blit.srcOffsets[0]				   = { 0, 0, 0 };
		blit.srcOffsets[1]				   = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel	   = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount	   = 1;
		blit.dstOffsets[0]				   = { 0, 0, 0 };
		blit.dstOffsets[1]				   = { mipWidth > 1 ? mipWidth / 2 : 1,
							   mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel	   = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount	   = 1;

		vkCmdBlitImage(commandBuffer, properties._image,
					   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, properties._image,
					   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
					   VK_FILTER_LINEAR);

		barrier.oldLayout	  = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout	  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
							 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
							 nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1)
			mipWidth /= 2;
		if (mipHeight > 1)
			mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = properties._mipLevels - 1;
	barrier.oldLayout	  = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout	  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
						 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
						 0, nullptr, 1, &barrier);

	deviceBackend.endSingleTimeCommands(
		deviceBackend._device, properties._commandPool,
		properties._graphicsQueue, commandBuffer);
}
