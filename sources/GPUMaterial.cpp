/*
** ETIB PROJECT, 2026
** evan
** File description:
** GPUGPUMaterial
*/

#include "evan/GPUMaterial.hpp"

#include <stb_image.h>

#include "evan/Renderer.hpp"

evan::GPUMaterial::GPUMaterial(std::shared_ptr<DeviceContext> deviceContext,
							   const Renderer &renderer,
							   const utility::graphic::Material &material,
							   uint32_t shaderID)
	: _shaderID(shaderID)
{
	this->getLogger().info()
		<< "Initializing GPUMaterial with shader ID: " << shaderID << "...";

	auto deviceBackend = deviceContext->getDeviceBackend();
	auto textures	   = material.getTextures();
	for (const auto &texture: textures) {
		GPUTexture::TextureType textureType;

		switch (texture->_type) {
			case utility::graphic::Texture::TextureType::Albedo:
				textureType = GPUTexture::TextureType::Albedo;
				break;
			case utility::graphic::Texture::TextureType::Normal:
				textureType = GPUTexture::TextureType::Normal;
				break;
			case utility::graphic::Texture::TextureType::Roughness:
				textureType = GPUTexture::TextureType::Roughness;
				break;
			case utility::graphic::Texture::TextureType::FontAtlas:
				textureType = GPUTexture::TextureType::FontAtlas;
				break;
			default:
				this->getLogger().warning()
					<< "Unknown texture type for material with shader ID: "
					<< shaderID << ". Defaulting to Albedo.";
				textureType = GPUTexture::TextureType::Albedo;
		}
		_textures.emplace_back(std::make_shared<GPUTexture>(
			*deviceContext, *texture, textureType));
	}

	this->createDescriptorSets(
		deviceBackend->_device, renderer.getDescriptorSetLayout(),
		renderer.getDescriptorPool(), renderer.getUniformBuffers());

	_uploadedVersion = material.getVersion();
}

evan::GPUMaterial::~GPUMaterial()
{
	this->getLogger().info() << "Destroying GPUMaterial...";
}

////////////////////
// Public Methods //
////////////////////

void evan::GPUMaterial::destroy(VkDevice device)
{
	this->getLogger().info() << "Destroying GPUMaterial...";
}

void evan::GPUMaterial::update(std::shared_ptr<DeviceContext> deviceContext,
							   const Renderer &renderer,
							   const utility::graphic::Material &material,
							   uint32_t shaderID)
{
	if (material.getVersion() == _uploadedVersion) {
		this->getLogger().info()
			<< "GREP No update needed for GPUMaterial with shader ID: "
			<< shaderID << " and material version: " << material.getVersion()
			<< ". Material version is unchanged.";
		return;
	}

	auto deviceBackend = deviceContext->getDeviceBackend();
	auto textures	   = material.getTextures();

	_textures.clear();

	for (const auto &texture: textures) {
		GPUTexture::TextureType textureType;

		switch (texture->_type) {
			case utility::graphic::Texture::TextureType::Albedo:
				textureType = GPUTexture::TextureType::Albedo;
				break;
			case utility::graphic::Texture::TextureType::Normal:
				textureType = GPUTexture::TextureType::Normal;
				break;
			case utility::graphic::Texture::TextureType::Roughness:
				textureType = GPUTexture::TextureType::Roughness;
				break;
			case utility::graphic::Texture::TextureType::FontAtlas:
				textureType = GPUTexture::TextureType::FontAtlas;
				break;
			default:
				this->getLogger().warning()
					<< "Unknown texture type for material with shader ID: "
					<< shaderID << ". Defaulting to Albedo.";
				textureType = GPUTexture::TextureType::Albedo;
		}
		_textures.emplace_back(std::make_shared<GPUTexture>(
			*deviceContext, *texture, textureType));
	}

	vkFreeDescriptorSets(deviceBackend->_device, renderer.getDescriptorPool(),
						 static_cast<uint32_t>(_descriptorSets.size()),
						 _descriptorSets.data());
	this->createDescriptorSets(
		deviceBackend->_device, renderer.getDescriptorSetLayout(),
		renderer.getDescriptorPool(), renderer.getUniformBuffers());

	_uploadedVersion = material.getVersion();
}

/////////////
// Getters //
/////////////

std::vector<VkDescriptorSet> evan::GPUMaterial::getDescriptorSets() const
{
	return _descriptorSets;
}

uint32_t evan::GPUMaterial::getShaderID() const
{
	return _shaderID;
}

uint32_t evan::GPUMaterial::getUploadedVersion() const
{
	return _uploadedVersion;
}

///////////////////////
// Protected methods //
///////////////////////

void evan::GPUMaterial::createDescriptorSets(
	VkDevice logicalDevice, VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	const std::vector<VkBuffer> &uniformBuffers)
{
	this->getLogger().info()
		<< "Creating descriptor sets for GPUMaterial with shader ID: "
		<< _shaderID << "...";
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
											   descriptorSetLayout);

	this->getLogger().info()
		<< "Setting up descriptor set allocation info with "
		<< MAX_FRAMES_IN_FLIGHT << " frames...";

	VkDescriptorSetAllocateInfo allocInfo {};
	allocInfo.sType			 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts		 = layouts.data();

	_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(logicalDevice, &allocInfo,
								 _descriptorSets.data())
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to allocate descriptor sets !";
		return;
	}

	this->getLogger().info()
		<< "Descriptor sets allocated successfully. Configuring descriptor "
		   "sets for shader access...";

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		this->getLogger().info()
			<< "Configuring descriptor set " << i
			<< " for GPUMaterial with shader ID: " << _shaderID << "...";
		std::vector<VkWriteDescriptorSet> descriptorWrites;

		VkDescriptorBufferInfo bufferInfo {};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range  = sizeof(Frame::UniformBufferObject);

		VkWriteDescriptorSet uboWrite {};
		uboWrite.sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uboWrite.dstSet			 = _descriptorSets[i];
		uboWrite.dstBinding		 = 0;
		uboWrite.descriptorType	 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboWrite.descriptorCount = 1;
		uboWrite.pBufferInfo	 = &bufferInfo;

		this->getLogger().info()
			<< "Adding uniform buffer descriptor write for descriptor set " << i
			<< "...";

		descriptorWrites.push_back(uboWrite);

		std::vector<VkDescriptorImageInfo> imageInfos;
		imageInfos.reserve(_textures.size());

		this->getLogger().info()
			<< "Adding texture descriptor writes for " << _textures.size()
			<< " textures in descriptor set " << i << "...";

		for (const auto &texture: _textures) {
			this->getLogger().info()
				<< "Configuring descriptor write for texture of type "
				<< static_cast<int>(texture->type) << " in descriptor set " << i
				<< "...";

			VkDescriptorImageInfo imageInfo {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView	  = texture->view;
			imageInfo.sampler	  = texture->sampler;

			imageInfos.push_back(imageInfo);

			VkWriteDescriptorSet write {};
			write.sType			  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet		  = _descriptorSets[i];
			write.dstBinding	  = getBinding(texture->type);
			write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorCount = 1;
			write.pImageInfo	  = &imageInfos.back();

			this->getLogger().info()
				<< "Adding descriptor write for texture of type "
				<< static_cast<int>(texture->type) << " in descriptor set " << i
				<< "...";
			descriptorWrites.push_back(write);
		}

		this->getLogger().info()
			<< "Updating descriptor sets for GPUMaterial with shader ID: "
			<< _shaderID << "...";

		vkUpdateDescriptorSets(logicalDevice,
							   static_cast<uint32_t>(descriptorWrites.size()),
							   descriptorWrites.data(), 0, nullptr);

		this->getLogger().info() << "Descriptor sets updated successfully for "
									"GPUMaterial with shader ID: "
								 << _shaderID << "...";
	}
}

/////////////////////
// Private Methods //
/////////////////////

uint32_t evan::GPUMaterial::getBinding(GPUTexture::TextureType type)
{
	this->getLogger().info() << "Getting descriptor binding for texture type: "
							 << static_cast<int>(type) << "...";
	switch (type) {
		case GPUTexture::TextureType::Albedo:
			this->getLogger().info()
				<< "Returning binding 1 for Albedo texture.";
			return 1;
		case GPUTexture::TextureType::Normal:
			this->getLogger().info()
				<< "Returning binding 2 for Normal texture.";
			return 2;
		case GPUTexture::TextureType::Roughness:
			this->getLogger().info()
				<< "Returning binding 3 for Roughness texture.";
			return 3;
		case GPUTexture::TextureType::FontAtlas:
			this->getLogger().info()
				<< "Returning binding 4 for FontAtlas texture.";
			return 4;
	}
	this->getLogger().warning()
		<< "Unknown texture type: " << static_cast<int>(type)
		<< ". Returning default binding 0.";
	return 0;	 // Default binding for unknown texture types
}
