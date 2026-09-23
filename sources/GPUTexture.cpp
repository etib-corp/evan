/*
** ETIB PROJECT, 2026
** xider
** File description:
** GPUTexture
*/

#include "evan/GPUTexture.hpp"

#include <algorithm>
#include <cmath>

evan::GPUTexture::GPUTexture(std::shared_ptr<DeviceContext> deviceContext,
							 const utility::graphic::Texture &texture,
							 TextureType type, const SamplerSettings &settings)
	: _deviceContext(deviceContext)
	, type(type)
{
	this->getLogger().info() << "Creating GPUTexture...";

	auto deviceBackend = deviceContext->getDeviceBackend();
	auto commandPool   = deviceContext->getCommandPool();
	auto graphicsQueue = deviceContext->getGraphicsQueue();

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
		this->createSampler(*deviceBackend, VkSamplerCreateInfo {}, settings);
	}
}

evan::GPUTexture::~GPUTexture()
{
	this->getLogger().info() << "Destroying GPUTexture...";
	this->cleanup();
}

////////////////////
// Public methods //
////////////////////

void evan::GPUTexture::destroy(VkDevice device)
{
	(void)device;
	this->cleanup();
}

///////////////////////
// Protected methods //
///////////////////////

void evan::GPUTexture::cleanup()
{
	if (!_deviceContext || !_deviceContext->getDeviceBackend()) {
		return;
	}

	// Block until pending uploads/mipmap generation referencing this image
	// have completed before destroying it.
	_deviceContext->getTransferManager().waitIdle();

	VkDevice device = _deviceContext->getDeviceBackend()->getDevice();

	this->getLogger().info() << "Destroying image view...";
	if (view != VK_NULL_HANDLE) {
		vkDestroyImageView(device, view, nullptr);
		view = VK_NULL_HANDLE;
	}

	if (_image != VK_NULL_HANDLE) {
		vkDestroyImage(device, _image, nullptr);
		_image = VK_NULL_HANDLE;
	}

	if (_memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, _memory, nullptr);
		_memory = VK_NULL_HANDLE;
	}

	if (sampler != VK_NULL_HANDLE) {
		vkDestroySampler(device, sampler, nullptr);
		sampler = VK_NULL_HANDLE;
	}
}

void evan::GPUTexture::createImage(const ADeviceBackend &deviceBackend,
								   const utility::graphic::Texture &texture,
								   VkCommandPool commandPool,
								   VkQueue graphicsQueue)
{
	this->getLogger().info() << "Creating Vulkan image for GPUTexture...";

	uint32_t texWidth	  = texture.width();
	uint32_t texHeight	  = texture.height();
	const uint8_t *pixels = texture.pixels().data();

	this->getLogger().info()
		<< "Texture dimensions: " << texWidth << "x" << texHeight;

	VkDeviceSize imageSize = texWidth * texHeight
		* (texture.type() == utility::graphic::Texture::TextureType::FontAtlas
			   ? 1
			   : 4);

	this->getLogger().info()
		<< "Calculated image size: " << imageSize << " bytes";

	if (texture.pixels().size() == 0) {
		this->getLogger().warning() << "Texture pixel data is empty. Creating "
									   "a 1x1 white texture as a fallback.";
		uint8_t whitePixel[4] = { 255, 255, 255, 255 };	   // RGBA for white
		pixels				  = whitePixel;
		imageSize			  = sizeof(whitePixel);
		texWidth			  = 1;
		texHeight			  = 1;
	}

	if (texture.type() == utility::graphic::Texture::TextureType::FontAtlas) {
		_mipLevel = 1;	  // Font atlases typically don't use mipmaps
	} else {
		_mipLevel = computeMipLevels(texWidth, texHeight);
	}
	this->getLogger().info() << "Calculated mip levels: " << _mipLevel;

	VkFormat imageFormat = type == TextureType::FontAtlas
		? VK_FORMAT_R8_UNORM
		: VK_FORMAT_R8G8B8A8_SRGB;

	ADeviceBackend::CreateImageProperties imageProperties = {
		._width		 = (uint32_t)texWidth,
		._height	 = (uint32_t)texHeight,
		._mipLevels	 = _mipLevel,
		._numSamples = VK_SAMPLE_COUNT_1_BIT,
		._format	 = imageFormat,
		._tiling	 = VK_IMAGE_TILING_OPTIMAL,
		._usage		 = VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		._properties  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._image		  = _image,
		._imageMemory = _memory
	};
	this->getLogger().info() << "Creating image...";
	deviceBackend.createImage(imageProperties);

	auto &transfer = _deviceContext->getTransferManager();

	this->getLogger().info() << "Recording image layout transition...";
	transfer.transitionImageLayout(_image, imageFormat,
								   VK_IMAGE_LAYOUT_UNDEFINED,
								   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								   _mipLevel);

	VkBufferImageCopy region {};
	region.bufferOffset					  = 0;
	region.bufferRowLength				  = 0;
	region.bufferImageHeight			  = 0;
	region.imageSubresource.aspectMask	  = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel	  = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount	  = 1;
	region.imageOffset					  = { 0, 0, 0 };
	region.imageExtent = { texWidth, texHeight, 1 };

	this->getLogger().info() << "Recording pixel upload...";
	transfer.uploadBufferToImage(pixels, imageSize, _image, region);

	GenerateMipmapsProperties propertiesMipmap = {
		._commandPool	= commandPool,
		._graphicsQueue = graphicsQueue,
		._image			= _image,
		._imageFormat	= imageFormat,
		._texWidth		= (uint32_t)texWidth,
		._texHeight		= (uint32_t)texHeight,
		._mipLevels		= _mipLevel
	};
	this->generateMipmaps(propertiesMipmap, deviceBackend);
}

void evan::GPUTexture::createImageView(const ADeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Creating image view...";
	view = deviceBackend
			   .createImageView(_image,
								type == TextureType::FontAtlas
									? VK_FORMAT_R8_UNORM
									: VK_FORMAT_R8G8B8A8_SRGB,
								VK_IMAGE_ASPECT_COLOR_BIT, _mipLevel)
			   .value;
}

void evan::GPUTexture::createSampler(const ADeviceBackend &deviceBackend,
									 VkSamplerCreateInfo samplerInfo,
									 const SamplerSettings &settings)
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
	vkGetPhysicalDeviceProperties(deviceBackend.getPhysicalDevice(),
								  &properties);

	// Check if samplerInfo is "empty" by testing its sType field.
	// If sType is not set, it's likely uninitialized.
	if (samplerInfo.sType == 0) {
		samplerInfo = this->getDefaultSamplerInfo(properties);
		this->applySamplerSettings(samplerInfo, settings);
	}

	// Enforce the hardware anisotropy limit for every sampler configuration.
	if (samplerInfo.anisotropyEnable == VK_TRUE) {
		samplerInfo.maxAnisotropy =
			(std::min)(samplerInfo.maxAnisotropy,
					   properties.limits.maxSamplerAnisotropy);
	}

	if (vkCreateSampler(deviceBackend.getDevice(), &samplerInfo, nullptr,
						&sampler)
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
	(void)deviceBackend;

	this->getLogger().info() << "Generating mipmaps for GPUTexture...";

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(
		_deviceContext->getDeviceBackend()->getPhysicalDevice(),
		properties._imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures
		  & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		this->getLogger().error()
			<< "Texture image format does not support linear blitting!";
		return;
	}

	_deviceContext->getTransferManager().generateMipmaps(
		properties._image, properties._imageFormat, properties._texWidth,
		properties._texHeight, properties._mipLevels);

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
	samplerInfo.maxLod					= computeMaxLod(_mipLevel);

	return samplerInfo;
}

uint32_t evan::GPUTexture::computeMipLevels(uint32_t width, uint32_t height)
{
	const uint32_t maxDimension = (std::max)(width, height);

	if (maxDimension == 0) {
		return 1;
	}
	return static_cast<uint32_t>(std::floor(std::log2(maxDimension))) + 1;
}

float evan::GPUTexture::computeMaxLod(uint32_t mipLevels)
{
	if (mipLevels == 0) {
		return 0.0f;
	}
	return static_cast<float>(mipLevels - 1);
}

void evan::GPUTexture::applySamplerSettings(VkSamplerCreateInfo &samplerInfo,
											const SamplerSettings &settings)
{
	samplerInfo.minLod			 = settings.minLod;
	samplerInfo.anisotropyEnable = settings.anisotropyEnable;
	samplerInfo.addressModeU	 = settings.addressModeU;
	samplerInfo.addressModeV	 = settings.addressModeV;
	samplerInfo.addressModeW	 = settings.addressModeW;

	if (settings.maxLod >= 0.0f) {
		samplerInfo.maxLod = settings.maxLod;
	}
	if (settings.maxAnisotropy > 0.0f) {
		samplerInfo.maxAnisotropy = settings.maxAnisotropy;
	}
}
