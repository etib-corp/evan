/*
** ETIB PROJECT, 2026
** evan
** File description:
** IPlatform
*/

#pragma once

#include "evan/EvanPlatform.hpp"
#include "evan/Error.hpp"

#include <utility/event/event.hpp>

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include <filesystem>
#include <string>

namespace evan
{
	class ADeviceBackend;
	class DeviceContext;
	class ASwapchainContext;

	/**
	 * @brief Abstract interface for platform-specific functionality.
	 *
	 * Defines a common interface for platform implementations. Derived classes
	 * must implement platform-specific behavior.
	 */
	class IPlatform:
		protected utility::logging::Loggable<IPlatform,
											 utility::logging::DefaultLogger>
	{
		public:
		/**
		 * @brief Virtual destructor.
		 *
		 * Ensures proper cleanup of derived class resources.
		 */
		virtual ~IPlatform() = default;

		/**
		 * @brief Get the required instance extensions for the platform.
		 *
		 * @return A vector of strings representing the required instance
		 * extensions.
		 */
		virtual std::vector<std::string>
			getRequiredInstanceExtensions() const = 0;

		/**
		 * @brief Get platform-specific Vulkan instance creation flags.
		 *
		 * @return Flags to OR into VkInstanceCreateInfo::flags.
		 *
		 * Platforms that require special instance creation flags (e.g. the
		 * portability enumeration bit on macOS/MoltenVK) override this method.
		 */
		virtual VkInstanceCreateFlags getInstanceCreateFlags() const
		{
			return 0;
		}

		/**
		 * @brief Get platform-specific required Vulkan device extensions.
		 *
		 * @return A vector of strings representing the required device
		 * extensions.
		 *
		 * Platforms that require additional device extensions (e.g. the
		 * portability subset on macOS/MoltenVK) override this method.
		 */
		virtual std::vector<std::string> getRequiredDeviceExtensions() const
		{
			return {};
		}

		/**
		 * @brief Get the path where the Vulkan pipeline cache is persisted.
		 *
		 * The returned path is where the cache blob written by
		 * `PipelineCache::persist()` is stored, and where the next run looks for
		 * a blob to reuse, so pipelines are not recompiled from scratch on every
		 * launch.
		 *
		 * The default implementation honors two environment variables and then
		 * falls back to the per-user cache directory of the running platform:
		 * - `EVAN_PIPELINE_CACHE_DISABLE`, set to any non-empty value other than
		 *   `0`, disables persistence entirely.
		 * - `EVAN_PIPELINE_CACHE_DIR` overrides the cache root directory, which
		 *   lets an application keep the blob next to its own build output.
		 *
		 * Platforms without a writable cache location return an empty path, in
		 * which case the pipeline cache is kept in memory only.
		 *
		 * @return The path of the pipeline cache blob, or an empty path to
		 * disable persistence.
		 */
		virtual std::filesystem::path getPipelineCachePath() const;

		/**
		 * @brief Check if the platform should close.
		 *
		 * @return true if the platform should close, false otherwise.
		 */
		virtual bool shouldClose() const = 0;

		/**
		 * @brief Poll for platform events.
		 *
		 * This method should be called regularly to process platform events.
		 *
		 * @param deviceBackend The device backend to use for event processing,
		 * if needed.
		 * @return A vector of shared pointers to Event objects representing the
		 * events that were polled from the platform. Each Event object contains
		 * information about the type of event, such as input events.
		 */
		virtual std::vector<std::shared_ptr<utility::event::Event>>
			pollEvents(ADeviceBackend &deviceBackend) = 0;

		/**
		 * @brief Create a device context for the platform.
		 *
		 * @return A shared pointer to the created device context.
		 *
		 * This method should create and return a device context that is
		 * compatible with the platform's requirements and capabilities.
		 */
		virtual std::shared_ptr<ADeviceBackend> createDeviceBackend() const = 0;

		/**
		 * @brief Get platform-specific instance creation information.
		 *
		 * @param deviceContext The device context to use for creating
		 * the swapchain context.
		 *
		 * @return A pointer to an XrBaseInStructure containing instance
		 * creation information, or nullptr if not applicable.
		 *
		 * This method can be used to provide additional instance creation
		 * information specific to the platform, such as application info or
		 * extension-specific structures.
		 */
		virtual std::shared_ptr<ASwapchainContext> createSwapchainContext(
			const DeviceContext &deviceContext) const = 0;

		/**
		 * @brief Get the last error recorded by the platform.
		 *
		 * Platforms record a sticky error while polling events (e.g.
		 * evan::Error::RuntimeLost when the OpenXR session is lost).
		 *
		 * @return The last normalized error recorded by the platform.
		 */
		Error getLastError() const
		{
			return _lastError;
		}

		protected:
		/**
		 * @brief Get the platform-specific cache root directory.
		 *
		 * This is the directory the pipeline cache blob is stored under, before
		 * the `EVAN_PIPELINE_CACHE_DIR` override is applied. It is where the
		 * platform is allowed to write user data, for example `~/Library/Caches`
		 * on macOS, `XDG_CACHE_HOME` or `~/.cache` on Linux, `%LOCALAPPDATA%` on
		 * Windows, and the application cache directory on Android.
		 *
		 * Every concrete platform implements this method, because only the
		 * platform knows which directory it is allowed to write to. Returning an
		 * empty path disables persistence.
		 *
		 * @return The cache root directory, or an empty path when the platform
		 * has no writable cache location.
		 */
		virtual std::filesystem::path getDefaultCacheRoot() const = 0;

		/**
		 * @brief Sticky error recorded by the platform while polling events.
		 */
		Error _lastError = Error::Ok;
	};
}	 // namespace evan