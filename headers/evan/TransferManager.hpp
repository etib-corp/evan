/*
** ETIB PROJECT, 2026
** evan
** File description:
** TransferManager
*/

#pragma once

#include "evan/EvanPlatform.hpp"

#include <cstdint>
#include <vector>

namespace evan
{
	/**
	 * @brief Owns staging memory and command buffers used to upload data to
	 * device-local resources without draining the graphics queue.
	 *
	 * Uploads are recorded into a single transfer command buffer and only
	 * submitted when flush() is called. Submission is asynchronous: a fence is
	 * stored per batch and the staging ring is only reused once that fence has
	 * signaled. This removes the per-copy vkQueueWaitIdle stall previously
	 * performed by ADeviceBackend::endSingleTimeCommands.
	 */
	class TransferManager
	{
		public:
		TransferManager() = default;
		~TransferManager();

		TransferManager(const TransferManager &) = delete;
		TransferManager &operator=(const TransferManager &) = delete;

		/**
		 * @brief Initializes the transfer queue and command pool.
		 *
		 * @param device The Vulkan logical device.
		 * @param physicalDevice The Vulkan physical device.
		 * @param graphicsQueue The queue used to submit transfer work. In this
		 * iteration transfers are submitted to the graphics queue so uploads
		 * are ordered before draw submissions without cross-queue
		 * synchronization.
		 * @param graphicsFamily The queue family index of graphicsQueue.
		 */
		void init(VkDevice device, VkPhysicalDevice physicalDevice,
				  VkQueue graphicsQueue, uint32_t graphicsFamily);

		/**
		 * @brief Releases every Vulkan resource owned by this manager.
		 */
		void destroy();

		/**
		 * @brief Copies host data into the staging ring and records a
		 * vkCmdCopyBuffer from the ring slot into dst.
		 */
		void uploadBuffer(const void *data, VkDeviceSize size, VkBuffer dst,
						  VkDeviceSize dstOffset = 0);

		/**
		 * @brief Copies host data into the staging ring and records a
		 * vkCmdCopyBufferToImage from the ring slot into dst.
		 */
		void uploadBufferToImage(const void *data, VkDeviceSize size,
								 VkImage dst,
								 const VkBufferImageCopy &region);

		/**
		 * @brief Records an image layout transition into the current batch.
		 */
		void transitionImageLayout(VkImage image, VkFormat format,
								   VkImageLayout oldLayout,
								   VkImageLayout newLayout,
								   uint32_t mipLevels);

		/**
		 * @brief Records a mipmap generation chain into the current batch.
		 */
		void generateMipmaps(VkImage image, VkFormat format, uint32_t width,
							 uint32_t height, uint32_t mipLevels);

		/**
		 * @brief Submits the pending transfer command buffer, if any.
		 */
		void flush();

		/**
		 * @brief Flushes pending work and blocks until every submitted batch
		 * has completed on the GPU.
		 */
		void waitIdle();

		private:
		struct PendingBatch {
			VkFence fence = VK_NULL_HANDLE;
			VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		};

		void ensureBegan();
		void recycle();
		void createStagingBuffer(VkDeviceSize size);

		VkDevice _device = VK_NULL_HANDLE;
		VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
		VkQueue _transferQueue = VK_NULL_HANDLE;
		uint32_t _transferFamily = 0;
		VkCommandPool _commandPool = VK_NULL_HANDLE;
		VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;
		bool _recording = false;
		bool _dirty = false;

		VkBuffer _stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory _stagingMemory = VK_NULL_HANDLE;
		void *_stagingMapped = nullptr;
		VkDeviceSize _stagingSize = 0;
		VkDeviceSize _stagingOffset = 0;

		std::vector<PendingBatch> _pending;
	};
}	 // namespace evan
