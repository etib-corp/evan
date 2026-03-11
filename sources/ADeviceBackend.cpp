/*
** EPITECH PROJECT, 2026
** evan
** File description:
** ADeviceBackend
*/

#include "ADeviceBackend.hpp"

evan::ADeviceBackend::~ADeviceBackend()
{
	vkDestroyDevice(_device, nullptr);
	vkDestroyInstance(_VkInstance, nullptr);
}

std::vector<VkLayerProperties> evan::ADeviceBackend::getAvailableLayers()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	return availableLayers;
}

void evan::ADeviceBackend::createBuffer(const CreateBufferProperties &properties) const
{
	VkBufferCreateInfo bufferInfo {};
	bufferInfo.sType	   = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size		   = properties._size;
	bufferInfo.usage	   = properties._usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(properties._logicalDevice, &bufferInfo, nullptr,
					   &properties._buffer)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(properties._logicalDevice, properties._buffer,
								  &memRequirements);

	VkMemoryAllocateInfo allocInfo {};
	allocInfo.sType			  = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize  = memRequirements.size;
	allocInfo.memoryTypeIndex = this->findMemoryType(
		properties._physicalDevice, memRequirements.memoryTypeBits,
		properties._properties);

	if (vkAllocateMemory(properties._logicalDevice, &allocInfo, nullptr,
						 &properties._bufferMemory)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(properties._logicalDevice, properties._buffer,
					   properties._bufferMemory, 0);
}

uint32_t evan::ADeviceBackend::findMemoryType(VkPhysicalDevice physicalDevice,
											   uint32_t typeFilter,
											   VkMemoryPropertyFlags properties) const
{
	VkPhysicalDeviceMemoryProperties memProperties;

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i))
			&& (memProperties.memoryTypes[i].propertyFlags & properties)
				== properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

void evan::ADeviceBackend::createImage(const CreateImageProperties &properties) const
{
	VkImageCreateInfo imageInfo {};
	imageInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType		= VK_IMAGE_TYPE_2D;
	imageInfo.extent.width	= properties._width;
	imageInfo.extent.height = properties._height;
	imageInfo.extent.depth	= 1;
	imageInfo.mipLevels		= 1;
	imageInfo.arrayLayers	= 1;
	imageInfo.format		= properties._format;
	imageInfo.tiling		= properties._tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage			= properties._usage;
	imageInfo.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.mipLevels		= properties._mipLevels;
	imageInfo.samples		= properties._numSamples;

	if (vkCreateImage(properties._logicalDevice, &imageInfo, nullptr,
					  &properties._image)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(properties._logicalDevice, properties._image,
								 &memRequirements);

	VkMemoryAllocateInfo allocInfo {};
	allocInfo.sType			  = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize  = memRequirements.size;
	allocInfo.memoryTypeIndex = this->findMemoryType(
		properties._physicalDevice, memRequirements.memoryTypeBits,
		properties._properties);

	if (vkAllocateMemory(properties._logicalDevice, &allocInfo, nullptr,
						 &properties._imageMemory)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory!");
	}

	vkBindImageMemory(properties._logicalDevice, properties._image,
					  properties._imageMemory, 0);
}