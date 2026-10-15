/*
** ETIB PROJECT, 2026
** evan
** File description:
** FrameStats
*/

#pragma once

#include "evan/EvanPlatform.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>

namespace evan
{
	/**
	 * @brief One accumulated CPU phase, in nanoseconds.
	 *
	 * Samples are added per frame; the report window keeps the most recent
	 * value, the sum and the sample count so the average can be computed when
	 * the window is reported.
	 */
	struct PhaseStats {
		/**
		 * @brief Most recent sample, in nanoseconds.
		 */
		uint64_t lastNs = 0;

		/**
		 * @brief Sum of the samples of the current window, in nanoseconds.
		 */
		uint64_t totalNs = 0;

		/**
		 * @brief Number of samples of the current window.
		 */
		uint32_t samples = 0;

		/**
		 * @brief Records one sample.
		 *
		 * @param ns Duration of the sample, in nanoseconds.
		 */
		void add(uint64_t ns) noexcept
		{
			lastNs = ns;
			totalNs += ns;
			++samples;
		}

		/**
		 * @brief Starts a new window.
		 *
		 * Keeps @c lastNs (the most recent value stays readable) and clears
		 * the sum and the sample count.
		 */
		void resetWindow() noexcept
		{
			totalNs = 0;
			samples = 0;
		}

		/**
		 * @brief Average duration of the window.
		 *
		 * @return The average in nanoseconds, or 0 when the window has no
		 * sample.
		 */
		[[nodiscard]] uint64_t averageNs() const noexcept
		{
			return samples == 0 ? 0 : totalNs / samples;
		}
	};

	/**
	 * @brief RAII timer feeding one PhaseStats.
	 *
	 * Constructed through FrameStats::timer(), which hands out a disabled
	 * timer (null sink) when statistics are off, so the disabled cost at the
	 * call site is a single predictable branch and no clock read.
	 */
	class ScopedTimer
	{
		public:
		/**
		 * @brief Starts measuring into @p phase.
		 *
		 * @param phase Destination phase, or nullptr to disable the timer.
		 */
		explicit ScopedTimer(PhaseStats *phase) noexcept
			: _phase(phase)
			, _start(phase != nullptr ? Clock::now() : TimePoint {})
		{
		}

		/**
		 * @brief Adds the elapsed time to the destination phase.
		 */
		~ScopedTimer() noexcept
		{
			if (_phase == nullptr) {
				return;
			}
			_phase->add(static_cast<uint64_t>(
				std::chrono::duration_cast<std::chrono::nanoseconds>(
					Clock::now() - _start)
					.count()));
		}

		ScopedTimer(const ScopedTimer &)			= delete;
		ScopedTimer &operator=(const ScopedTimer &) = delete;
		ScopedTimer(ScopedTimer &&)					= delete;
		ScopedTimer &operator=(ScopedTimer &&)		= delete;

		private:
		using Clock		= std::chrono::steady_clock;
		using TimePoint = Clock::time_point;

		PhaseStats *_phase;	   ///< Destination phase, null when disabled.
		TimePoint _start;	   ///< Start of the measured window.
	};

	/**
	 * @brief Per-frame instrumentation data (CPU phases and counters).
	 *
	 * Disabled by default: @c enabled mirrors the EVAN_STATS environment
	 * variable and every hook collapses to one branch when it is false. When
	 * enabled, samples and counters accumulate in fixed-size members (no heap
	 * traffic on the hot path) until the report window is flushed by the
	 * renderer (one aggregated log record every EVAN_STATS_INTERVAL frames).
	 */
	struct FrameStats {
		/**
		 * @brief Whether CPU timers and the interval log are active.
		 */
		bool enabled = false;

		/**
		 * @brief Whether GPU timestamp queries were created and are readable.
		 *
		 * False when EVAN_STATS_GPU is off or the device does not support
		 * timestamp queries; the report then prints the GPU columns as
		 * unavailable.
		 */
		bool gpuAvailable = false;

		// Frame-level CPU phases, one sample per frame each.
		PhaseStats transferFlush;	 ///< Renderer::drawFrame: upload flush.
		PhaseStats preprocess;		 ///< OpenXR frame preprocess.
		PhaseStats inFlightWait;	 ///< Slot-reuse fence wait.
		PhaseStats acquire;			 ///< Swapchain image acquire (all sets).
		PhaseStats updateUniform;	 ///< Per-view uniform buffer updates.
		PhaseStats record;			 ///< Per-view command recording.
		PhaseStats submit;			 ///< Per-view queue submissions.
		PhaseStats renderWait;		 ///< OpenXR end-of-frame fence wait.
		PhaseStats present;			 ///< Present/composition (all sets).
		PhaseStats postprocess;		 ///< OpenXR frame postprocess.

		// Per-view-slot detail, indexed like the frame's view slots.
		std::array<PhaseStats, MAX_SWAPCHAINS>
			updateUniformView;	  ///< updateUniform, per view slot.
		std::array<PhaseStats, MAX_SWAPCHAINS>
			recordView;	   ///< record, per view slot.
		std::array<PhaseStats, MAX_SWAPCHAINS>
			submitView;	   ///< submit, per view slot.
		std::array<PhaseStats, MAX_SWAPCHAINS>
			gpuPass;	///< GPU pass time, per view slot.

		// Counters, accumulated over the current window.
		uint64_t frames			  = 0;	  ///< Frames in the window.
		uint64_t totalMeshes	  = 0;	  ///< Meshes registered at draw time.
		uint64_t visibleMeshes	  = 0;	  ///< Meshes surviving culling.
		uint64_t culledMeshes	  = 0;	  ///< Meshes removed by culling.
		uint64_t skippedMeshes	  = 0;	  ///< Meshes skipped (bad state).
		uint64_t drawCalls		  = 0;	  ///< vkCmdDrawIndexed(Indirect) count.
		uint64_t instancedDraws	  = 0;	  ///< Draws with instanceCount > 1.
		uint64_t pipelineBinds	  = 0;	  ///< vkCmdBindPipeline count.
		uint64_t descriptorBinds  = 0;	  ///< vkCmdBindDescriptorSets count.
		uint64_t syncCalls		  = 0;	  ///< RessourceManager::sync() calls.
		uint64_t syncSkips		  = 0;	  ///< sync() calls short-circuited.
		uint64_t meshCreations	  = 0;	  ///< GPUMesh constructions.
		uint64_t meshDestructions = 0;	  ///< GPUMesh destructions.
		uint64_t uploadWaits	  = 0;	  ///< Blocking upload fence waits.

		/**
		 * @brief Creates a scoped timer for @p phase.
		 *
		 * @param phase The phase to accumulate into.
		 * @return A disabled timer when statistics are off, an active one
		 * otherwise.
		 */
		[[nodiscard]] ScopedTimer timer(PhaseStats &phase) noexcept
		{
			return ScopedTimer(enabled ? &phase : nullptr);
		}

		/**
		 * @brief Starts a new report window.
		 *
		 * Clears every phase window and counter; keeps @c enabled and
		 * @c gpuAvailable.
		 */
		void resetWindow() noexcept;
	};

	/**
	 * @brief Process-wide counter plumbing for the frame statistics.
	 *
	 * The counters live here because their increment sites (RessourceManager,
	 * GPUMesh, TransferManager, device backends) have no access to a
	 * Renderer's FrameStats. The renderer folds the deltas into its window
	 * once per frame through takeGlobalDeltas(). Increments are gated behind
	 * the EVAN_STATS flag, so with statistics off each site is one
	 * predictable branch.
	 */
	namespace stats
	{
		/**
		 * @brief Whether the global counters currently record.
		 *
		 * @return True when EVAN_STATS is set, unless a test overrode the
		 * gate.
		 */
		[[nodiscard]] bool enabled() noexcept;

		/**
		 * @brief Overrides the counter gate.
		 *
		 * Unit-test hook only: the override is never set by engine code. Pass
		 * std::nullopt to fall back to the EVAN_STATS environment variable.
		 *
		 * @param state Forced gate value, or std::nullopt for the default.
		 */
		void setEnabledForTesting(std::optional<bool> state) noexcept;

		/**
		 * @brief Counts one RessourceManager::sync() call.
		 */
		void countSync() noexcept;

		/**
		 * @brief Counts one sync() call short-circuited on the version check.
		 */
		void countSyncSkip() noexcept;

		/**
		 * @brief Counts one GPUMesh construction.
		 */
		void countMeshCreation() noexcept;

		/**
		 * @brief Counts one GPUMesh destruction.
		 */
		void countMeshDestruction() noexcept;

		/**
		 * @brief Counts one blocking upload wait (staging or one-time
		 * submission fence).
		 */
		void countUploadWait() noexcept;

		/**
		 * @brief Folds the global counter deltas into @p stats.
		 *
		 * Adds (current value - last taken value) to the matching counters
		 * and moves the snapshot forward, so every increment is accounted
		 * exactly once whichever frame reports it.
		 *
		 * @param stats The window to update.
		 */
		void takeGlobalDeltas(FrameStats &stats) noexcept;

		/**
		 * @brief Masks a raw timestamp to the device's valid bits.
		 *
		 * Devices whose timestamp counter is narrower than 64 bits leave the
		 * high bits undefined; only the low
		 * VkPhysicalDeviceLimits::timestampValidBits bits are meaningful.
		 *
		 * @param value Raw timestamp counter value.
		 * @param validBits Number of valid bits reported by the device.
		 * @return The masked value.
		 */
		[[nodiscard]] inline uint64_t maskTimestamp(uint64_t value,
													uint32_t validBits) noexcept
		{
			if (validBits >= 64) {
				return value;
			}
			if (validBits == 0) {
				return 0;
			}
			return value & ((uint64_t { 1 } << validBits) - 1);
		}

		/**
		 * @brief Converts a timestamp delta to nanoseconds.
		 *
		 * @param begin First timestamp, already masked.
		 * @param end Second timestamp, already masked.
		 * @param period Nanoseconds per tick
		 * (VkPhysicalDeviceLimits::timestampPeriod, may be fractional).
		 * @param deltaNs Receives the converted duration.
		 * @return False when the delta is unusable: the counter went
		 * backwards (wrap with a narrow counter) or the period is unknown.
		 */
		[[nodiscard]] inline bool timestampDeltaNs(uint64_t begin, uint64_t end,
												   float period,
												   uint64_t &deltaNs) noexcept
		{
			if (end < begin || !(period > 0.0f)) {
				return false;
			}
			deltaNs = static_cast<uint64_t>(static_cast<double>(end - begin)
											* static_cast<double>(period));
			return true;
		}
	}	 // namespace stats
}	 // namespace evan
