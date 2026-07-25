/*
** ETIB PROJECT, 2026
** xider
** File description:
** GPUTexture
*/

#include "evan/GPUTexture.hpp"

#include <algorithm>
#include <cmath>

evan::GPUTexture::GPUTexture(const DeviceContext &deviceContext,
							 const utility::graphic::Texture &texture,
							 TextureType type)
	: type(type)
{
	this->getLogger().info() << "Creating GPUTexture...";

	auto deviceBackend = deviceContext.getDeviceBackend();
	auto commandPool   = deviceContext.getCommandPool();
	auto graphicsQueue = deviceContext.getGraphicsQueue();

	this->createImage(*deviceBackend, texture, commandPool, graphicsQueue);
	this->createImageView(*deviceBackend);

	this->getLogger().info() << "Creating default sampler for GPUTexture...";
	if (type == TextureType::FontAtlas) {
		_mipLevel = 1;	  // Font atlases typically don't use mipmaps
		this->createSampler(
			*deviceBackend,
			VkSamplerCreateInfo {
				.sType			  = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
				.pNext			  = nullptr,
				.flags			  = 0,
				.magFilter		  = VK_FILTER_LINEAR,
				.minFilter		  = VK_FILTER_LINEAR,
				.mipmapMode		  = VK_SAMPLER_MIPMAP_MODE_LINEAR,
				.addressModeU	  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				.addressModeV	  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				.addressModeW	  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				.mipLodBias		  = 0.0f,
				.anisotropyEnable = VK_FALSE,
				.maxAnisotropy	  = 1.0f,
				.compareEnable	  = (VkBool32)VK_FALSE,
				.compareOp		  = VK_COMPARE_OP_ALWAYS,
				.minLod			  = 0.0f,
				.maxLod			  = 0.0f,
				.borderColor	  = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
				.unnormalizedCoordinates = VK_FALSE,
			});
	} else {
		this->createSampler(*deviceBackend, VkSamplerCreateInfo {});
	}
}

evan::GPUTexture::~GPUTexture()
{
	this->getLogger().info() << "Destroying GPUTexture...";
}

////////////////////
// Public methods //
////////////////////

void evan::GPUTexture::destroy(VkDevice device)
{
	this->getLogger().info() << "Destroying Vulkan resources for GPUTexture...";

	this->getLogger().info() << "Destroying image view...";
	vkDestroyImage(device, _image, nullptr);

	this->getLogger().info() << "Freeing image memory...";
	vkFreeMemory(device, _memory, nullptr);

	this->getLogger().info() << "Destroying sampler...";
	vkDestroySampler(device, sampler, nullptr);
}

///////////////////////
// Protected methods //
///////////////////////

void evan::GPUTexture::createImage(const ADeviceBackend &deviceBackend,
								   const utility::graphic::Texture &texture,
								   VkCommandPool commandPool,
								   VkQueue graphicsQueue)
{
	this->getLogger().info() << "Creating Vulkan image for GPUTexture...";

	uint32_t texWidth	  = texture.width();
	uint32_t texHeight	  = texture.height();
	const uint8_t *pixels = texture._pixels.data();

	this->getLogger().info()
		<< "Texture dimensions: " << texWidth << "x" << texHeight;

	VkDeviceSize imageSize = texWidth * texHeight
		* (texture._type == utility::graphic::Texture::TextureType::FontAtlas
			   ? 1
			   : 4);
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	this->getLogger().info()
		<< "Calculated image size: " << imageSize << " bytes";

	if (texture._pixels.size() == 0) {
		this->getLogger().warning() << "Texture pixel data is empty. Creating "
									   "a 1x1 white texture as a fallback.";
		uint8_t whitePixel[4] = { 255, 255, 255, 255 };	   // RGBA for white
		pixels				  = whitePixel;
		imageSize			  = sizeof(whitePixel);
		texWidth			  = 1;
		texHeight			  = 1;
	}

	if (texture._type == utility::graphic::Texture::TextureType::FontAtlas) {
		_mipLevel = 1;	  // Font atlases typically don't use mipmaps
	} else {
		_mipLevel = static_cast<uint32_t>(
						std::floor(std::log2((std::max)(texWidth, texHeight))))
			+ 1;
	}
	this->getLogger().info() << "Calculated mip levels: " << _mipLevel;

	ADeviceBackend::CreateBufferProperties stagingBufferProperties = {
		._size		 = imageSize,
		._usage		 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		._properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		._buffer	   = stagingBuffer,
		._bufferMemory = stagingBufferMemory
	};

	deviceBackend.createBuffer(stagingBufferProperties);

	void *data;

	this->getLogger().info()
		<< "Mapping staging buffer memory and copying pixel data...";
	vkMapMemory(deviceBackend._device, stagingBufferMemory, 0, imageSize, 0,
				&data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));

	this->getLogger().info() << "Unmapping staging buffer memory...";
	vkUnmapMemory(deviceBackend._device, stagingBufferMemory);

	ADeviceBackend::CreateImageProperties imageProperties = {
		._width		 = (uint32_t)texWidth,
		._height	 = (uint32_t)texHeight,
		._mipLevels	 = _mipLevel,
		._numSamples = VK_SAMPLE_COUNT_1_BIT,
		._format	 = type == TextureType::FontAtlas ? VK_FORMAT_R8_UNORM
													  : VK_FORMAT_R8G8B8A8_SRGB,
		._tiling	 = VK_IMAGE_TILING_OPTIMAL,
		._usage		 = VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		._properties  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._image		  = _image,
		._imageMemory = _memory
	};
	this->getLogger().info() << "Creating image...";
	deviceBackend.createImage(imageProperties);

	ADeviceBackend::TransitionImageLayoutProperties transitionProperties = {
		._commandPool	= commandPool,
		._graphicsQueue = graphicsQueue,
		._image			= _image,
		._format		= type == TextureType::FontAtlas ? VK_FORMAT_R8_UNORM
														 : VK_FORMAT_R8G8B8A8_SRGB,
		._oldLayout		= VK_IMAGE_LAYOUT_UNDEFINED,
		._newLayout		= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		._mipLevels		= _mipLevel
	};
	deviceBackend.transitionImageLayout(transitionProperties);

	ADeviceBackend::CopyBufferToImageProperties copyProperties = {
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
		._imageFormat	= type == TextureType::FontAtlas
			? VK_FORMAT_R8_UNORM
			: VK_FORMAT_R8G8B8A8_SRGB,
		._texWidth		= (uint32_t)texWidth,
		._texHeight		= (uint32_t)texHeight,
		._mipLevels		= _mipLevel
	};
	this->generateMipmaps(propertiesMipmap, deviceBackend);

	this->getLogger().info() << "Cleaning up staging buffer...";
	vkDestroyBuffer(deviceBackend._device, stagingBuffer, nullptr);
	vkFreeMemory(deviceBackend._device, stagingBufferMemory, nullptr);
}

void evan::GPUTexture::createImageView(const ADeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Creating image view...";
	view = deviceBackend.createImageView(_image,
										 type == TextureType::FontAtlas
											 ? VK_FORMAT_R8_UNORM
											 : VK_FORMAT_R8G8B8A8_SRGB,
										 VK_IMAGE_ASPECT_COLOR_BIT, _mipLevel);
}

void evan::GPUTexture::createSampler(const ADeviceBackend &deviceBackend,
									 VkSamplerCreateInfo samplerInfo)
{
	auto samplerInfoStr =
		"Sampler info - sType: " + std::to_string(samplerInfo.sType)
		+ ", magFilter: " + std::to_string(samplerInfo.magFilter)
		+ ", minFilter: " + std::to_string(samplerInfo.minFilter)
		+ ", addressModeU: " + std::to_string(samplerInfo.addressModeU)
		+ ", addressModeV: " + std::to_string(samplerInfo.addressModeV)
		+ ", addressModeW: " + std::to_string(samplerInfo.addressModeW);

	this->getLogger().info() << "Creating sampler...";
	this->getLogger().info()
		<< "Sampler info provided: " << (samplerInfo.sType != 0);
	if (samplerInfo.sType != 0) {
		this->getLogger().info() << "Using provided sampler info.";
		this->getLogger().info() << "Sampler info: " << samplerInfoStr;
	} else {
		this->getLogger().info()
			<< "No sampler info provided. Using default values.";
	}

	VkPhysicalDeviceProperties properties {};
	vkGetPhysicalDeviceProperties(deviceBackend._physicalDevice, &properties);

	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	// Check if samplerInfo is "empty" by testing its sType field.
	// If sType is not set, it's likely uninitialized.
	if (samplerInfo.sType == 0) {
		samplerInfo = this->getDefaultSamplerInfo(properties);
	}

	if (vkCreateSampler(deviceBackend._device, &samplerInfo, nullptr, &sampler)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to create texture sampler!";
		return;
	}
}

/////////////////////
// Private methods //
/////////////////////

void evan::GPUTexture::generateMipmaps(
	const GenerateMipmapsProperties &properties,
	const ADeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Generating mipmaps for GPUTexture...";

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(deviceBackend._physicalDevice,
										properties._imageFormat,
										&formatProperties);

	if (!(formatProperties.optimalTilingFeatures
		  & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		this->getLogger().error()
			<< "Texture image format does not support linear blitting!";
		return;
	}

	VkCommandBuffer commandBuffer =
		deviceBackend.beginSingleTimeCommands(properties._commandPool);

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

	this->getLogger().info() << "Starting mipmap generation loop...";

	for (uint32_t i = 1; i < properties._mipLevels; i++) {
		this->getLogger().info()
			<< "Generating mip level " << i << " with dimensions " << mipWidth
			<< "x" << mipHeight;

		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout	  = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout	  = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		this->getLogger().info() << "Transitioning mip level " << i - 1
								 << " to TRANSFER_SRC_OPTIMAL for blitting...";
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
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1,
							   mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel	   = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount	   = 1;

		this->getLogger().info() << "Blitting from mip level " << i - 1
								 << " to mip level " << i << "...";
		vkCmdBlitImage(commandBuffer, properties._image,
					   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, properties._image,
					   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
					   VK_FILTER_LINEAR);

		barrier.oldLayout	  = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout	  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		this->getLogger().info() << "Transitioning mip level " << i - 1
								 << " to SHADER_READ_ONLY_OPTIMAL...";
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
							 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
							 nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1) {
			this->getLogger().info() << "Halving mip width for next level...";
			mipWidth /= 2;
		}
		if (mipHeight > 1) {
			this->getLogger().info() << "Halving mip height for next level...";
			mipHeight /= 2;
		}
	}

	barrier.subresourceRange.baseMipLevel = properties._mipLevels - 1;
	barrier.oldLayout	  = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout	  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	this->getLogger().info()
		<< "Transitioning final mip level " << properties._mipLevels - 1
		<< " to SHADER_READ_ONLY_OPTIMAL...";
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
						 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
						 0, nullptr, 1, &barrier);
	deviceBackend.endSingleTimeCommands(
		properties._commandPool, properties._graphicsQueue, commandBuffer);
	this->getLogger().info() << "Mipmap generation completed.";
}

VkSamplerCreateInfo evan::GPUTexture::getDefaultSamplerInfo(
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
