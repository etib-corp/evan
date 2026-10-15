/*
 Copyright (c) 2025 ETIB Corporation

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#pragma once

#include <utility/system_io/android_system_io.hpp>
#include <utility/system_io/default_system_io.hpp>

// GLM configuration
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

/*
 * @brief Validation layers for Vulkan debugging.
 *
 * This constant defines the validation layers used for debugging Vulkan
 * applications. These layers provide additional checks and validation
 * during development to help identify issues and improve code quality.
 *
 */
const std::vector<const char *> validationLayers = {
	"VK_LAYER_KHRONOS_validation"	 // Khronos validation layer
};

/*
 * @brief Maximum number of frames in flight.
 *
 * This constant defines the maximum number of frames that can be in flight
 * at any given time. It is used to manage synchronization and resource
 * allocation for rendering operations in Vulkan.
 *
 */
const int MAX_FRAMES_IN_FLIGHT = 2;

/*
 * @brief Maximum number of swapchain image sets per frame.
 *
 * Used to size per-swapchain synchronization resources (fences and
 * semaphores) for a frame. Current backends use one (desktop) or two
 * (OpenXR, one per eye); a larger bound leaves room for multi-view.
 *
 */
const int MAX_SWAPCHAINS = 8;

/*
 * @brief Maximum number of views renderable in a single multiview pass.
 *
 * Stereo view configurations have two views; the multiview uniform layout and
 * the render-pass view mask are sized for this bound. View configurations with
 * more views fall back to the per-view recording path.
 */
constexpr uint32_t MAX_MULTIVIEW_VIEWS = 2;

/**
 * @brief How the multiview rendering path is selected.
 *
 * Driven by the EVAN_MULTIVIEW environment variable, mirroring EVAN_MSAA:
 * unset or unrecognised values mean Auto.
 */
enum class MultiviewMode {
	Auto,	 ///< Use multiview when the device and runtime support it.
	On,		 ///< Require multiview; log an error when it cannot be enabled.
	Off		 ///< Force the two-pass fallback, to test or compare it.
};

/**
 * @brief Reads the EVAN_MULTIVIEW override.
 *
 * Accepts 1/on/true/yes for MultiviewMode::On and 0/off/false/no for
 * MultiviewMode::Off. The value is read once per process.
 *
 * @return The requested multiview mode.
 */
inline MultiviewMode multiviewMode()
{
	static const MultiviewMode mode = [] {
		const char *value = std::getenv("EVAN_MULTIVIEW");
		if (value == nullptr || value[0] == '\0') {
			return MultiviewMode::Auto;
		}
		if (std::strcmp(value, "1") == 0 || std::strcmp(value, "on") == 0
			|| std::strcmp(value, "true") == 0
			|| std::strcmp(value, "yes") == 0) {
			return MultiviewMode::On;
		}
		if (std::strcmp(value, "0") == 0 || std::strcmp(value, "off") == 0
			|| std::strcmp(value, "false") == 0
			|| std::strcmp(value, "no") == 0) {
			return MultiviewMode::Off;
		}
		return MultiviewMode::Auto;
	}();
	return mode;
}

/**
 * @brief Parses a boolean EVAN_* flag value.
 *
 * A missing, empty, or "0"-leading value disables the flag; any other value
 * enables it, matching the EVAN_DEBUG_DRAW_LOG / EVAN_DEBUG_DISABLE_CULLING
 * convention.
 *
 * @param value Environment value to parse (may be null).
 * @return True when the value enables the flag.
 */
inline bool statsFlagEnabled(const char *value)
{
	return value != nullptr && value[0] != '\0' && value[0] != '0';
}

/**
 * @brief Parses the EVAN_STATS_INTERVAL frame count.
 *
 * Accepts a positive decimal count; missing, zero, negative, non-numeric or
 * absurdly large values fall back to the default report window of 120 frames
 * (about two seconds at 60 Hz).
 *
 * @param value Environment value to parse (may be null).
 * @return The report window, in frames.
 */
inline unsigned statsIntervalFromValue(const char *value)
{
	constexpr unsigned defaultInterval	= 120u;
	constexpr unsigned long maxInterval = 1000000ul;

	if (value == nullptr || value[0] == '\0') {
		return defaultInterval;
	}

	char *end				   = nullptr;
	const unsigned long parsed = std::strtoul(value, &end, 10);
	if (end == value || parsed == 0 || parsed > maxInterval) {
		return defaultInterval;
	}
	return static_cast<unsigned>(parsed);
}

/**
 * @brief Whether frame statistics (EVAN_STATS) are enabled for this process.
 *
 * Off by default: the timers and counters collapse to a single predictable
 * branch at every hook, and no frame data is ever logged. The value is read
 * once per process.
 *
 * @return True when EVAN_STATS is set to an enabling value.
 */
inline bool statsEnabled()
{
	static const bool enabled = statsFlagEnabled(std::getenv("EVAN_STATS"));
	return enabled;
}

/**
 * @brief Number of frames per statistics report window.
 *
 * Driven by EVAN_STATS_INTERVAL; defaults to 120 frames. The value is read
 * once per process.
 *
 * @return The report window, in frames.
 */
inline unsigned statsLogInterval()
{
	static const unsigned interval =
		statsIntervalFromValue(std::getenv("EVAN_STATS_INTERVAL"));
	return interval;
}

/**
 * @brief Whether GPU timestamp queries may be created (EVAN_STATS_GPU).
 *
 * On with EVAN_STATS by default; set EVAN_STATS_GPU=0 to keep the CPU
 * instrumentation without creating query pools, for example to attribute the
 * enabled cost of the GPU path. The value is read once per process.
 *
 * @return True when GPU timestamps are allowed.
 */
inline bool statsGpuEnabled()
{
	static const bool enabled = [] {
		const char *value = std::getenv("EVAN_STATS_GPU");
		if (value == nullptr || value[0] == '\0') {
			return true;
		}
		return value[0] != '0';
	}();
	return enabled;
}

/**
 * @brief Builds the render-pass view mask for a view count.
 *
 * Multiview is only used for stereo today: at least two views are required for
 * the mask to be non-zero, and counts above MAX_MULTIVIEW_VIEWS fall back to
 * the per-view path (mask 0).
 *
 * @param viewCount Number of views to render.
 * @return The view mask, or 0 when multiview must not be used.
 */
constexpr uint32_t multiviewViewMask(std::size_t viewCount)
{
	if (viewCount < 2
		|| viewCount > static_cast<std::size_t>(MAX_MULTIVIEW_VIEWS)) {
		return 0u;
	}
	return (1u << viewCount) - 1u;
}

/**
 * @brief Number of per-frame slots to allocate for a backend.
 *
 * Per-frame resources (command buffers, image-available/render-finished
 * semaphores and uniform/instance/indirect buffer slots) are sized from the
 * real number of view and swapchain slots the backend needs, instead of
 * always allocating MAX_SWAPCHAINS of them. Both counts are considered so
 * that command buffers (indexed by view slot) and semaphores (indexed by
 * swapchain slot) stay in range, including the future case where several
 * views share a single swapchain.
 *
 * MAX_SWAPCHAINS remains the hard upper bound; at least one slot is always
 * allocated so a frame stays usable if a backend reports no view yet.
 *
 * @param viewCount Number of views to render.
 * @param swapchainCount Number of swapchain image sets.
 * @return The clamped per-frame slot count, in [1, MAX_SWAPCHAINS].
 */
constexpr std::size_t frameSlotCount(std::size_t viewCount,
									 std::size_t swapchainCount)
{
	const std::size_t requested =
		viewCount > swapchainCount ? viewCount : swapchainCount;

	if (requested == 0) {
		return 1;
	}
	return requested > static_cast<std::size_t>(MAX_SWAPCHAINS)
		? static_cast<std::size_t>(MAX_SWAPCHAINS)
		: requested;
}

/*
 * @brief Initial number of per-instance transforms per view slot.
 *
 * Floor and initial allocation of the instancing vertex buffer: each frame
 * keeps one slot per view, each slot holding 4x4 model matrices fed to the
 * vertex shader as per-instance attributes. The buffer grows past this bound
 * on demand (see nextInstanceCapacity) so the constant is not a hard cap and
 * no mesh is ever dropped for lack of a transform slot.
 */
const int MAX_INSTANCES_PER_VIEW = 1024;

/**
 * @brief Computes the instance-buffer capacity to use for a request.
 *
 * Keeps @p current when it already covers @p required, and otherwise grows
 * geometrically from the larger of @p current and MAX_INSTANCES_PER_VIEW, so
 * the buffer is reallocated rarely instead of on every frame that gains a few
 * visible meshes. The result is never smaller than MAX_INSTANCES_PER_VIEW.
 *
 * @param required Number of transform slots the frame needs.
 * @param current Capacity the buffer currently has (0 when none exists yet).
 * @return The capacity to allocate, in
 * [MAX_INSTANCES_PER_VIEW, max(size_t)] and at least @p required.
 */
constexpr std::size_t nextInstanceCapacity(std::size_t required,
										   std::size_t current)
{
	const std::size_t minimum =
		static_cast<std::size_t>(MAX_INSTANCES_PER_VIEW);
	std::size_t capacity = current > minimum ? current : minimum;

	while (capacity < required) {
		// Saturate instead of overflowing when a request is absurdly large.
		if (capacity > std::numeric_limits<std::size_t>::max() / 2) {
			return required;
		}
		capacity *= 2;
	}
	return capacity;
}

/*
 * @brief Maximum number of indirect draw commands per view slot.
 *
 * Bounds the indirect draw buffer: each frame keeps one slot per view, each
 * slot holding up to this many VkDrawIndexedIndirectCommand entries used to
 * batch the sorted, surviving draws.
 */
const int MAX_INDIRECT_COMMANDS_PER_VIEW = 4096;

// Vulkan headers - needed for both GLFW and OpenXR
#if defined(__GLFW__) || defined(__OPENXR__)
	#include <vulkan/vulkan.h>
#endif

#include <vulkan/vulkan.h>

#ifdef __GLFW__
	#include <vulkan/vulkan.hpp>
	#define GLFW_INCLUDE_VULKAN
	#include <GLFW/glfw3.h>
	#include <GLFW/glfw3native.h>
#endif

#ifdef __OPENXR__
	#ifdef _WIN32
		#define NOMINMAX
		#include <windows.h>
	#endif
	#include <openxr/openxr.h>
	#ifdef __ANDROID__
		#include <jni.h>
	#endif
	#include <openxr/openxr_platform.h>
#endif

/*
 ** @brief Callback type for the debug messenger.
 **
 ** This callback is used to handle debug messages from the Vulkan
 * API.
 ** It is used to log the debug messages from the Vulkan API.
 **
 ** @param severity the severity of the message
 ** @param type the type of the message
 ** @param pCallbackData the callback data
 ** @param pUserData the user data
 **
 ** @return true if the message is handled, false otherwise
 */
typedef VkBool32 debugCallback_t(VkDebugUtilsMessageSeverityFlagBitsEXT,
								 VkDebugUtilsMessageTypeFlagsEXT,
								 const VkDebugUtilsMessengerCallbackDataEXT *,
								 void *);
