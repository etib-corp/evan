/*
** ETIB PROJECT, 2026
** evan
** File description:
** TransferManager
*/

#include "evan/TransferManager.hpp"

#include <cstring>

namespace
{
	constexpr VkDeviceSize kDefaultStagingSize = 64u * 1024u * 1024u;
	constexpr VkDeviceSize kStagingAlignment = 256u;

	VkDeviceSize alignUp(VkDeviceSize value, VkDeviceSize alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}

	uint32_t findMemoryType(VkPhysicalDevice physicalDevice,
							uint32_t typeFilter,
							VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties {};
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
			if ((typeFilter & (1u << i))
				&& (memProperties.memoryTypes[i].propertyFlags & properties)
					== properties) {
				return i;
			}
		}
		return 0;
	}
}	 // namespace

evan::TransferManager::~TransferManager()
{
	this->destroy();
}

void evan::TransferManager::init(VkDevice device,
								 VkPhysicalDevice physicalDevice,
								 VkQueue graphicsQueue,
								 uint32_t graphicsFamily)
{
	_device = device;
	_physicalDevice = physicalDevice;
	_transferQueue = graphicsQueue;
	_transferFamily = graphicsFamily;

	VkCommandPoolCreateInfo poolInfo {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = _transferFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool);

	this->createStagingBuffer(kDefaultStagingSize);
}

void evan::TransferManager::destroy()
{
	if (_device == VK_NULL_HANDLE) {
		return;
	}

	this->waitIdle();

	if (_stagingBuffer != VK_NULL_HANDLE) {
		vkUnmapMemory(_device, _stagingMemory);
		vkDestroyBuffer(_device, _stagingBuffer, nullptr);
		vkFreeMemory(_device, _stagingMemory, nullptr);
		_stagingBuffer = VK_NULL_HANDLE;
		_stagingMemory = VK_NULL_HANDLE;
		_stagingMapped = nullptr;
	}

	if (_commandPool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(_device, _commandPool, nullptr);
		_commandPool = VK_NULL_HANDLE;
	}

	_device = VK_NULL_HANDLE;
	_physicalDevice = VK_NULL_HANDLE;
	_transferQueue = VK_NULL_HANDLE;
}

void evan::TransferManager::ensureBegan()
{
	if (_recording) {
		return;
	}

	this->recycle();

	VkCommandBufferAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = _commandPool;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(_device, &allocInfo, &_commandBuffer);

	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(_commandBuffer, &beginInfo);

	_recording = true;
	_dirty = false;
}

void evan::TransferManager::recycle()
{
	for (auto it = _pending.begin(); it != _pending.end();) {
		VkResult status = vkGetFenceStatus(_device, it->fence);
		if (status == VK_SUCCESS) {
			vkDestroyFence(_device, it->fence, nullptr);
			vkFreeCommandBuffers(_device, _commandPool, 1,
								 &it->commandBuffer);
			it = _pending.erase(it);
		} else {
			++it;
		}
	}
}

void evan::TransferManager::createStagingBuffer(VkDeviceSize size)
{
	if (_stagingBuffer != VK_NULL_HANDLE) {
		vkUnmapMemory(_device, _stagingMemory);
		vkDestroyBuffer(_device, _stagingBuffer, nullptr);
		vkFreeMemory(_device, _stagingMemory, nullptr);
		_stagingBuffer = VK_NULL_HANDLE;
		_stagingMemory = VK_NULL_HANDLE;
		_stagingMapped = nullptr;
	}

	VkBufferCreateInfo bufferInfo {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vkCreateBuffer(_device, &bufferInfo, nullptr, &_stagingBuffer);

	VkMemoryRequirements memRequirements {};
	vkGetBufferMemoryRequirements(_device, _stagingBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(
		_physicalDevice, memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vkAllocateMemory(_device, &allocInfo, nullptr, &_stagingMemory);
	vkBindBufferMemory(_device, _stagingBuffer, _stagingMemory, 0);

	vkMapMemory(_device, _stagingMemory, 0, size, 0, &_stagingMapped);

	_stagingSize = size;
	_stagingOffset = 0;
}

void evan::TransferManager::uploadBuffer(const void *data, VkDeviceSize size,
										 VkBuffer dst, VkDeviceSize dstOffset)
{
	if (_device == VK_NULL_HANDLE || size == 0) {
		return;
	}

	VkDeviceSize aligned = alignUp(size, kStagingAlignment);
	if (aligned > _stagingSize
		|| _stagingOffset + aligned > _stagingSize) {
		this->waitIdle();
		if (aligned > _stagingSize) {
			this->createStagingBuffer(aligned);
		}
	}

	this->ensureBegan();

	VkDeviceSize offset = _stagingOffset;
	std::memcpy(static_cast<char *>(_stagingMapped) + offset, data,
				static_cast<std::size_t>(size));
	_stagingOffset += aligned;

	VkBufferCopy copyRegion {};
	copyRegion.srcOffset = offset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = size;
	vkCmdCopyBuffer(_commandBuffer, _stagingBuffer, dst, 1, &copyRegion);
	_dirty = true;
}

void evan::TransferManager::uploadBufferToImage(
	const void *data, VkDeviceSize size, VkImage dst,
	const VkBufferImageCopy &region)
{
	if (_device == VK_NULL_HANDLE || size == 0) {
		return;
	}

	VkDeviceSize aligned = alignUp(size, kStagingAlignment);
	if (aligned > _stagingSize
		|| _stagingOffset + aligned > _stagingSize) {
		this->waitIdle();
		if (aligned > _stagingSize) {
			this->createStagingBuffer(aligned);
		}
	}

	this->ensureBegan();

	VkDeviceSize offset = _stagingOffset;
	std::memcpy(static_cast<char *>(_stagingMapped) + offset, data,
				static_cast<std::size_t>(size));
	_stagingOffset += aligned;

	VkBufferImageCopy copyRegion = region;
	copyRegion.bufferOffset = offset;
	vkCmdCopyBufferToImage(_commandBuffer, _stagingBuffer, dst,
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
						   &copyRegion);
	_dirty = true;
}

void evan::TransferManager::transitionImageLayout(
	VkImage image, VkFormat format, VkImageLayout oldLayout,
	VkImageLayout newLayout, uint32_t mipLevels)
{
	if (_device == VK_NULL_HANDLE) {
		return;
	}

	this->ensureBegan();

	VkImageMemoryBarrier barrier {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
		&& newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			   && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
			   && newLayout
				   == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		return;
	}

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (format == VK_FORMAT_D32_SFLOAT_S8_UINT
			|| format == VK_FORMAT_D24_UNORM_S8_UINT) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	vkCmdPipelineBarrier(_commandBuffer, sourceStage, destinationStage, 0, 0,
						 nullptr, 0, nullptr, 1, &barrier);
	_dirty = true;
}

void evan::TransferManager::generateMipmaps(VkImage image, VkFormat format,
											uint32_t width, uint32_t height,
											uint32_t mipLevels)
{
	if (_device == VK_NULL_HANDLE || mipLevels <= 1) {
		return;
	}

	(void)format;
	this->ensureBegan();

	VkImageMemoryBarrier barrier {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = static_cast<int32_t>(width);
	int32_t mipHeight = static_cast<int32_t>(height);

	for (uint32_t i = 1; i < mipLevels; ++i) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
							 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
							 nullptr, 1, &barrier);

		VkImageBlit blit {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1,
							   mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(_commandBuffer, image,
					   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
					   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
					   VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
							 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
							 nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1) {
			mipWidth /= 2;
		}
		if (mipHeight > 1) {
			mipHeight /= 2;
		}
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
						 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
						 0, nullptr, 1, &barrier);
	_dirty = true;
}

void evan::TransferManager::flush()
{
	if (_device == VK_NULL_HANDLE || !_recording) {
		return;
	}

	if (!_dirty) {
		vkEndCommandBuffer(_commandBuffer);
		vkFreeCommandBuffers(_device, _commandPool, 1, &_commandBuffer);
		_commandBuffer = VK_NULL_HANDLE;
		_recording = false;
		return;
	}

	vkEndCommandBuffer(_commandBuffer);

	VkFenceCreateInfo fenceInfo {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fence = VK_NULL_HANDLE;
	vkCreateFence(_device, &fenceInfo, nullptr, &fence);

	VkSubmitInfo submitInfo {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffer;
	vkQueueSubmit(_transferQueue, 1, &submitInfo, fence);

	_pending.push_back({ fence, _commandBuffer });
	_commandBuffer = VK_NULL_HANDLE;
	_recording = false;
	_dirty = false;
}

void evan::TransferManager::waitIdle()
{
	if (_device == VK_NULL_HANDLE) {
		return;
	}

	this->flush();
	this->recycle();

	if (_pending.empty()) {
		_stagingOffset = 0;
		return;
	}

	std::vector<VkFence> fences;
	fences.reserve(_pending.size());
	for (const auto &batch: _pending) {
		fences.push_back(batch.fence);
	}

	vkWaitForFences(_device, static_cast<uint32_t>(fences.size()),
					fences.data(), VK_TRUE, UINT64_MAX);

	for (const auto &batch: _pending) {
		vkDestroyFence(_device, batch.fence, nullptr);
		vkFreeCommandBuffers(_device, _commandPool, 1, &batch.commandBuffer);
	}
	_pending.clear();
	_stagingOffset = 0;
}
