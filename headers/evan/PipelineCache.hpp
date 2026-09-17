/*
** ETIB PROJECT, 2026
** evan
** File description:
** PipelineCache
*/

#pragma once

#include "evan/EvanPlatform.hpp"

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include <cstddef>
#include <filesystem>
#include <vector>

namespace evan
{
	/**
	 * @brief Owns the Vulkan pipeline cache used when creating pipelines and
	 * persists it across runs.
	 *
	 * A `VkPipelineCache` lets the driver reuse the work it already did when
	 * compiling earlier pipelines: both for pipelines created later during
	 * the current run and for pipelines created by previous runs, when the
	 * cache is reloaded from disk. Without one, every launch recompiles every
	 * pipeline from scratch (`vkCreateGraphicsPipelines(..., VK_NULL_HANDLE,
	 * ...)`), which directly costs startup time.
	 *
	 * The persisted blob is a driver and device specific artifact. On startup
	 * the blob is validated against the vendor id, device id and
	 * `pipelineCacheUUID` reported by the physical device, so a blob produced
	 * by another device, or by another driver version, is silently discarded
	 * instead of being handed to the driver.
	 *
	 * @note The cache is a pure optimization: every failure (unreadable file,
	 * unwritable directory, corrupt or stale blob, driver rejection) is
	 * non-fatal and simply leaves the renderer compiling its pipelines
	 * uncached.
	 *
	 * @note The blob is written atomically (temporary file followed by a
	 * rename) so a process killed during shutdown can never leave a truncated
	 * file behind.
	 */
	class PipelineCache:
		protected utility::logging::Loggable<PipelineCache,
											 utility::logging::DefaultLogger>
	{
		public:
		PipelineCache() = default;

		~PipelineCache() = default;

		PipelineCache(const PipelineCache &) = delete;
		PipelineCache &operator=(const PipelineCache &) = delete;

		/**
		 * @brief Builds the cache key of a physical device.
		 *
		 * The key is the part of a pipeline cache blob that identifies the
		 * device and the driver version that produced it. It is stored at the
		 * beginning of every blob by the Vulkan implementation itself, which
		 * makes it usable both to write and to validate a persisted blob.
		 *
		 * @param properties Properties of the physical device the cache is
		 * created for.
		 * @return The header identifying that device and driver.
		 */
		[[nodiscard]] static VkPipelineCacheHeaderVersionOne
			buildHeader(const VkPhysicalDeviceProperties &properties);

		/**
		 * @brief Tells whether a blob can safely be handed to the driver.
		 *
		 * Compares the header stored at the beginning of @p blob with
		 * @p expected. A blob that is too short, whose header has an unknown
		 * version, or that was produced for another vendor, device or driver
		 * version is rejected.
		 *
		 * @param blob Raw bytes read from a previously persisted cache.
		 * @param expected Cache key of the current device.
		 * @return True when the blob matches the current device, false
		 * otherwise.
		 */
		[[nodiscard]] static bool isBlobCompatible(
			const std::vector<std::byte> &blob,
			const VkPipelineCacheHeaderVersionOne &expected);

		/**
		 * @brief Reads a whole pipeline cache blob from disk.
		 *
		 * @param path Path of the blob to read.
		 * @param blob Output vector receiving the raw bytes. It is cleared
		 * when the read fails.
		 * @return True when the file was read completely, false otherwise.
		 */
		[[nodiscard]] static bool
			readBlob(const std::filesystem::path &path,
					 std::vector<std::byte> &blob);

		/**
		 * @brief Writes a pipeline cache blob to disk atomically.
		 *
		 * Missing parent directories are created. The blob is first written
		 * to a temporary file next to @p path and then renamed over it, so a
		 * concurrent reader never observes a partially written cache.
		 *
		 * @param path Path of the blob to write.
		 * @param blob Raw bytes to persist.
		 * @return True when the blob was written, false otherwise.
		 */
		[[nodiscard]] static bool
			writeBlob(const std::filesystem::path &path,
					  const std::vector<std::byte> &blob);

		/**
		 * @brief Creates the Vulkan pipeline cache.
		 *
		 * When @p path holds a blob that still matches the physical device,
		 * the blob is loaded and handed to `vkCreatePipelineCache` so the
		 * driver can reuse previously compiled pipelines. In every other case
		 * an empty cache is created.
		 *
		 * @param device The logical device the cache belongs to.
		 * @param physicalDevice The physical device used to validate the
		 * persisted blob.
		 * @param path Path of the persisted blob, or an empty path to keep
		 * the cache in memory only.
		 *
		 * @note This method never reports failure. When the cache cannot be
		 * created at all, getHandle() keeps returning `VK_NULL_HANDLE`, which
		 * `vkCreateGraphicsPipelines` accepts as "no cache".
		 */
		void init(VkDevice device, VkPhysicalDevice physicalDevice,
				  const std::filesystem::path &path);

		/**
		 * @brief Flushes the pipeline cache to the path given to init().
		 *
		 * Does nothing when the cache is empty or when persistence is
		 * disabled. Call it before destroy() so the pipelines created during
		 * this run are available to the next one.
		 */
		void persist();

		/**
		 * @brief Destroys the Vulkan pipeline cache.
		 *
		 * @param device The logical device the cache was created from.
		 *
		 * @note Must be called after the pipelines created with the cache
		 * have been destroyed. This method does not persist the cache; call
		 * persist() first.
		 */
		void destroy(VkDevice device);

		/**
		 * @brief Retrieves the Vulkan handle to pass to
		 * `vkCreateGraphicsPipelines`.
		 *
		 * @return The pipeline cache handle, or `VK_NULL_HANDLE` when the
		 * cache could not be created.
		 */
		[[nodiscard]] VkPipelineCache getHandle() const { return _handle; }

		/**
		 * @brief Tells whether the cache is written to and read from disk.
		 *
		 * @return True when a path was given to init().
		 */
		[[nodiscard]] bool isPersistent() const { return !_path.empty(); }

		private:
		/**
		 * @brief The logical device the cache belongs to.
		 */
		VkDevice _device = VK_NULL_HANDLE;

		/**
		 * @brief The Vulkan pipeline cache handle.
		 *
		 * `VK_NULL_HANDLE` when the cache could not be created, which means
		 * pipelines are compiled without any cache.
		 */
		VkPipelineCache _handle = VK_NULL_HANDLE;

		/**
		 * @brief Path of the persisted blob, empty when persistence is
		 * disabled.
		 */
		std::filesystem::path _path;

		/**
		 * @brief Cache key of the device the cache was created for.
		 */
		VkPipelineCacheHeaderVersionOne _expectedHeader {};
	};
}	 // namespace evan
