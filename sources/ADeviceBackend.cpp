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

void evan::ADeviceBackend::createBuffer(
	const CreateBufferProperties &properties) const
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

uint32_t
	evan::ADeviceBackend::findMemoryType(VkPhysicalDevice physicalDevice,
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

void evan::ADeviceBackend::createImage(
	const CreateImageProperties &properties) const
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

VkImageView evan::ADeviceBackend::createImageView(
	VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const
{
	VkImageViewCreateInfo viewInfo {};
	viewInfo.sType	  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image	  = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format	  = format;
	viewInfo.subresourceRange.aspectMask	 = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel	 = 0;
	viewInfo.subresourceRange.levelCount	 = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount	 = 1;
	viewInfo.subresourceRange.levelCount	 = mipLevels;

	VkImageView imageView;
	if (vkCreateImageView(_device, &viewInfo, nullptr, &imageView)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

void evan::ADeviceBackend::transitionImageLayout(
	const TransitionImageLayoutProperties &properties) const
{
	VkCommandBuffer commandBuffer = this->beginSingleTimeCommands(
		properties._logicalDevice, properties._commandPool);

	VkImageMemoryBarrier barrier {};
	barrier.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout			= properties._oldLayout;
	barrier.newLayout			= properties._newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image				= properties._image;
	barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel	= 0;
	barrier.subresourceRange.levelCount		= 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount		= 1;
	barrier.subresourceRange.levelCount		= properties._mipLevels;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (properties._oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
		&& properties._newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (properties._oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			   && properties._newLayout
				   == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (properties._oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
			   && properties._newLayout
				   == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	if (properties._newLayout
		== VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (this->hasStencilComponent(properties._format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
						 nullptr, 0, nullptr, 1, &barrier);

	this->endSingleTimeCommands(properties._logicalDevice,
								properties._commandPool,
								properties._graphicsQueue, commandBuffer);
}

VkCommandBuffer evan::ADeviceBackend::beginSingleTimeCommands(
	VkDevice logicalDevice, VkCommandPool commandPool) const
{
	VkCommandBufferAllocateInfo allocInfo {};
	allocInfo.sType		  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level		  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer)
		!= VK_SUCCESS) {
		std::cerr << "Failed to allocate command buffer" << std::endl;
		return VK_NULL_HANDLE;
	}

	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

bool evan::ADeviceBackend::hasStencilComponent(VkFormat format) const
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT
		|| format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void evan::ADeviceBackend::endSingleTimeCommands(
	VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue,
	VkCommandBuffer commandBuffer) const
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo {};
	submitInfo.sType			  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers	  = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

void evan::ADeviceBackend::copyBufferToImage(
	const CopyBufferToImageProperties &properties) const
{
	VkCommandBuffer commandBuffer = this->beginSingleTimeCommands(
		properties._logicalDevice, properties._commandPool);

	VkBufferImageCopy region {};
	region.bufferOffset					   = 0;
	region.bufferRowLength				   = 0;
	region.bufferImageHeight			   = 0;
	region.imageSubresource.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel	   = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount	   = 1;
	region.imageOffset					   = { 0, 0, 0 };
	region.imageExtent = { properties._width, properties._height, 1 };

	vkCmdCopyBufferToImage(commandBuffer, properties._buffer, properties._image,
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	this->endSingleTimeCommands(properties._logicalDevice,
								properties._commandPool,
								properties._graphicsQueue, commandBuffer);
}

void evan::ADeviceBackend::copyBuffer(const CopyBufferProperties &properties) const
{
	VkCommandBuffer commandBuffer = this->beginSingleTimeCommands(
		properties._logicalDevice, properties._commandPool);

	VkBufferCopy copyRegion {};
	copyRegion.size = properties._size;
	vkCmdCopyBuffer(commandBuffer, properties._srcBuffer, properties._dstBuffer,
					1, &copyRegion);

	this->endSingleTimeCommands(properties._logicalDevice,
								 properties._commandPool,
								 properties._graphicsQueue, commandBuffer);
}
