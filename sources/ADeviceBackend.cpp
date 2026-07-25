/*
** ETIB PROJECT, 2026
** evan
** File description:
** ADeviceBackend
*/

#include "evan/ADeviceBackend.hpp"

////////////////////
// Public Methods //
////////////////////

evan::ADeviceBackend::~ADeviceBackend()
{
}

void evan::ADeviceBackend::createBuffer(
	const CreateBufferProperties &properties) const
{
	VkBufferCreateInfo bufferInfo {};
	bufferInfo.sType	   = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size		   = properties._size;
	bufferInfo.usage	   = properties._usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	this->getLogger().info()
		<< "Creating buffer with size: " << properties._size
		<< " and usage flags: " << properties._usage;
	if (vkCreateBuffer(_device, &bufferInfo, nullptr, &properties._buffer)
		!= VK_SUCCESS) {
		this->getLogger().error()
			<< "Failed to create buffer with size: " << properties._size
			<< " and usage flags: " << properties._usage;
		return;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(_device, properties._buffer,
								  &memRequirements);

	VkMemoryAllocateInfo allocInfo {};
	allocInfo.sType			  = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize  = memRequirements.size;
	allocInfo.memoryTypeIndex = this->findMemoryType(
		memRequirements.memoryTypeBits, properties._properties);

	this->getLogger().info()
		<< "Allocating buffer memory with size: " << memRequirements.size
		<< " and memory type index: " << allocInfo.memoryTypeIndex;
	if (vkAllocateMemory(_device, &allocInfo, nullptr,
						 &properties._bufferMemory)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to allocate buffer memory!";
		return;
	}

	this->getLogger().info() << "Successfully allocated buffer memory!";
	this->getLogger().info() << "Binding buffer to memory...";
	vkBindBufferMemory(_device, properties._buffer, properties._bufferMemory,
					   0);
	this->getLogger().info() << "Successfully bound buffer to memory!";
}

void evan::ADeviceBackend::transitionImageLayout(
	const TransitionImageLayoutProperties &properties) const
{
	this->getLogger().info()
		<< "Transitioning image layout from " << properties._oldLayout << " to "
		<< properties._newLayout;
	VkCommandBuffer commandBuffer =
		this->beginSingleTimeCommands(properties._commandPool);

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

	this->getLogger().info()
		<< "Setting up image memory barrier for layout transition...";
	this->getLogger().info()
		<< "Old layout: " << barrier.oldLayout
		<< ", new layout: " << barrier.newLayout
		<< ", image: " << (uintptr_t)barrier.image
		<< ", aspect mask: " << barrier.subresourceRange.aspectMask
		<< ", base mip level: " << barrier.subresourceRange.baseMipLevel
		<< ", level count: " << barrier.subresourceRange.levelCount
		<< ", base array layer: " << barrier.subresourceRange.baseArrayLayer
		<< ", layer count: " << barrier.subresourceRange.layerCount;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (properties._oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
		&& properties._newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		this->getLogger().info() << "Transitioning from undefined layout to "
									"transfer destination optimal layout...";
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (properties._oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			   && properties._newLayout
				   == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		this->getLogger().info()
			<< "Transitioning from transfer destination optimal layout to "
			   "shader read-only optimal layout...";
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (properties._oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
			   && properties._newLayout
				   == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		this->getLogger().info()
			<< "Transitioning from undefined layout to depth-stencil "
			   "attachment optimal layout...";
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		this->getLogger().error() << "unsupported layout transition!";
		return;
	}

	if (properties._newLayout
		== VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		this->getLogger().info()
			<< "Setting aspect mask for depth-stencil image...";

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (this->hasStencilComponent(properties._format)) {
			this->getLogger().info()
				<< "Image format has stencil component, adding stencil aspect "
				   "to aspect mask...";
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		this->getLogger().info() << "Setting aspect mask for color image...";
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	this->getLogger().info() << "Submitting pipeline barrier...";

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
						 nullptr, 0, nullptr, 1, &barrier);

	this->getLogger().info() << "Successfully submitted pipeline barrier!";

	this->endSingleTimeCommands(properties._commandPool,
								properties._graphicsQueue, commandBuffer);
}

VkCommandBuffer evan::ADeviceBackend::beginSingleTimeCommands(
	VkCommandPool commandPool) const
{
	this->getLogger().info() << "Beginning single time command buffer...";

	VkCommandBufferAllocateInfo allocInfo {};
	allocInfo.sType		  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level		  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffer;

	this->getLogger().info()
		<< "Allocating command buffer with " << allocInfo.commandBufferCount
		<< " command buffer(s) from command pool: " << (uintptr_t)commandPool;

	if (vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to allocate command buffer";
		return VK_NULL_HANDLE;
	}

	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	this->getLogger().info()
		<< "Beginning command buffer recording using one-time submit flag...";

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	this->getLogger().info() << "Successfully began recording command buffer!";
	return commandBuffer;
}

void evan::ADeviceBackend::endSingleTimeCommands(
	VkCommandPool commandPool, VkQueue graphicsQueue,
	VkCommandBuffer commandBuffer) const
{
	this->getLogger().info() << "Ending single time command buffer...";

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo {};
	submitInfo.sType			  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers	  = &commandBuffer;

	this->getLogger().info() << "Submitting command buffer...";

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

	this->getLogger().info() << "Waiting for queue to become idle...";

	vkQueueWaitIdle(graphicsQueue);

	this->getLogger().info() << "Freeing command buffer...";

	vkFreeCommandBuffers(_device, commandPool, 1, &commandBuffer);

	this->getLogger().info()
		<< "Successfully ended single time command buffer!";
}

void evan::ADeviceBackend::createImage(
	const CreateImageProperties &properties) const
{
	this->getLogger().info()
		<< "Creating image with:\n width: " << properties._width
		<< ",\n height: " << properties._height
		<< ",\n mip levels: " << properties._mipLevels
		<< ",\n num samples: " << properties._numSamples
		<< ",\n format: " << properties._format
		<< ",\n tiling: " << properties._tiling
		<< ",\n usage flags: " << properties._usage
		<< ",\n memory properties: " << properties._properties;

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

	this->getLogger().info()
		<< "Image creation info:\n width: " << imageInfo.extent.width
		<< ",\n height: " << imageInfo.extent.height
		<< ",\n mip levels: " << imageInfo.mipLevels
		<< ",\n num samples: " << imageInfo.samples
		<< ",\n format: " << imageInfo.format
		<< ",\n tiling: " << imageInfo.tiling
		<< ",\n usage flags: " << imageInfo.usage
		<< ",\n memory properties: " << properties._properties;

	this->getLogger().info() << "Creating image...";

	if (vkCreateImage(_device, &imageInfo, nullptr, &properties._image)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to create image!";
		return;
	}

	this->getLogger().info() << "Successfully created image!";

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(_device, properties._image, &memRequirements);

	VkMemoryAllocateInfo allocInfo {};
	allocInfo.sType			  = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize  = memRequirements.size;
	allocInfo.memoryTypeIndex = this->findMemoryType(
		memRequirements.memoryTypeBits, properties._properties);

	this->getLogger().info()
		<< "Allocating image memory with size: " << memRequirements.size
		<< " and memory type index: " << allocInfo.memoryTypeIndex;

	this->getLogger().info() << "Allocating image memory...";

	if (vkAllocateMemory(_device, &allocInfo, nullptr, &properties._imageMemory)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to allocate image memory!";
		return;
	}

	this->getLogger().info() << "Successfully allocated image memory!";
	this->getLogger().info() << "Binding image to memory...";

	vkBindImageMemory(_device, properties._image, properties._imageMemory, 0);

	this->getLogger().info() << "Successfully bound image to memory!";
}

void evan::ADeviceBackend::copyBufferToImage(
	const CopyBufferToImageProperties &properties) const
{
	this->getLogger().info()
		<< "Copying buffer to image with:\n width: " << properties._width
		<< ",\n height: " << properties._height;

	VkCommandBuffer commandBuffer =
		this->beginSingleTimeCommands(properties._commandPool);

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

	this->getLogger().info()
		<< "Region for buffer to image copy:\n buffer offset: "
		<< region.bufferOffset
		<< ",\n buffer row length: " << region.bufferRowLength
		<< ",\n buffer image height: " << region.bufferImageHeight
		<< ",\n image aspect mask: " << region.imageSubresource.aspectMask
		<< ",\n image mip level: " << region.imageSubresource.mipLevel
		<< ",\n image base array layer: "
		<< region.imageSubresource.baseArrayLayer
		<< ",\n image layer count: " << region.imageSubresource.layerCount
		<< ",\n image offset: (" + std::to_string(region.imageOffset.x) + ", "
			+ std::to_string(region.imageOffset.y) + ", "
			+ std::to_string(region.imageOffset.z) + ")"
		<< ",\n image extent: (" + std::to_string(region.imageExtent.width)
			+ ", " + std::to_string(region.imageExtent.height) + ", "
			+ std::to_string(region.imageExtent.depth) + ")";
	this->getLogger().info() << "Copying buffer to image...";

	vkCmdCopyBufferToImage(commandBuffer, properties._buffer, properties._image,
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	this->getLogger().info() << "Successfully copied buffer to image!";

	this->endSingleTimeCommands(properties._commandPool,
								properties._graphicsQueue, commandBuffer);
}

void evan::ADeviceBackend::copyBuffer(
	const CopyBufferProperties &properties) const
{
	this->getLogger().info()
		<< "Copying buffer with size: " << properties._size;
	VkCommandBuffer commandBuffer =
		this->beginSingleTimeCommands(properties._commandPool);

	VkBufferCopy copyRegion {};
	copyRegion.size = properties._size;
	vkCmdCopyBuffer(commandBuffer, properties._srcBuffer, properties._dstBuffer,
					1, &copyRegion);

	this->getLogger().info() << "Successfully copied buffer!";

	this->endSingleTimeCommands(properties._commandPool,
								properties._graphicsQueue, commandBuffer);
}

VkImageView
	evan::ADeviceBackend::createImageView(VkImage image, VkFormat format,
										  VkImageAspectFlags aspectFlags,
										  uint32_t mipLevels) const
{
	this->getLogger().info() << "Creating image view with format: " << format
							 << ", aspect flags: " << aspectFlags
							 << ", and mip levels: " << mipLevels;

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

	this->getLogger().info()
		<< "Image view creation info:\n format: " << viewInfo.format
		<< ", aspect flags: " << viewInfo.subresourceRange.aspectMask
		<< ", mip levels: " << viewInfo.subresourceRange.levelCount
		<< ", base mip level: " << viewInfo.subresourceRange.baseMipLevel
		<< ", layer count: " << viewInfo.subresourceRange.layerCount
		<< ", base array layer: " << viewInfo.subresourceRange.baseArrayLayer;
	this->getLogger().info() << "Creating image view...";

	if (vkCreateImageView(_device, &viewInfo, nullptr, &imageView)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to create texture image view!";
		return VK_NULL_HANDLE;
	}

	this->getLogger().info() << "Successfully created image view!";

	return imageView;
}

///////////////////////
// Protected methods //
///////////////////////

std::vector<VkLayerProperties> evan::ADeviceBackend::getAvailableLayers()
{
	this->getLogger().info() << "Enumerating available Vulkan layers...";

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	this->getLogger().info()
		<< "Found " << layerCount << " available Vulkan layer(s).";

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	this->getLogger().info()
		<< "Successfully enumerated available Vulkan layers.";

	return availableLayers;
}

/////////////////////
// Private Methods //
/////////////////////

bool evan::ADeviceBackend::hasStencilComponent(VkFormat format) const
{
	this->getLogger().info()
		<< "Checking if format " << format << " has stencil component...";
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT
		|| format == VK_FORMAT_D24_UNORM_S8_UINT;
}

uint32_t
	evan::ADeviceBackend::findMemoryType(uint32_t typeFilter,
										 VkMemoryPropertyFlags properties) const
{
	this->getLogger().info()
		<< "Finding suitable memory type for type filter: " << typeFilter
		<< " and memory property flags: " << properties;

	VkPhysicalDeviceMemoryProperties memProperties;

	vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i))
			&& (memProperties.memoryTypes[i].propertyFlags & properties)
				== properties) {
			this->getLogger().info()
				<< "Found suitable memory type at index: " << i;
			return i;
		}
	}

	this->getLogger().error() << "Failed to find suitable memory type!";
	this->getLogger().warning()
		<< "Returning 0 as fallback, but this may lead to undefined behavior!";
	return 0;
}
