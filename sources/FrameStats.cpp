/*
** ETIB PROJECT, 2026
** evan
** File description:
** FrameStats
*/

#include "evan/FrameStats.hpp"

#include <atomic>

namespace
{
	std::atomic<uint64_t> gSyncCalls { 0 };
	std::atomic<uint64_t> gSyncSkips { 0 };
	std::atomic<uint64_t> gMeshCreations { 0 };
	std::atomic<uint64_t> gMeshDestructions { 0 };
	std::atomic<uint64_t> gUploadWaits { 0 };

	/**
	 * @brief Last taken values of the global counters.
	 *
	 * takeGlobalDeltas() computes (current - snapshot), so every increment is
	 * accounted exactly once whichever frame reports it.
	 */
	struct GlobalSnapshot {
		uint64_t syncCalls		  = 0;
		uint64_t syncSkips		  = 0;
		uint64_t meshCreations	  = 0;
		uint64_t meshDestructions = 0;
		uint64_t uploadWaits	  = 0;
	};

	GlobalSnapshot gSnapshot;

	/**
	 * @brief Gate override for tests: -1 follows EVAN_STATS, 0/1 force it.
	 */
	std::atomic<int> gEnabledOverride { -1 };
}	 // namespace

void evan::FrameStats::resetWindow() noexcept
{
	transferFlush.resetWindow();
	preprocess.resetWindow();
	inFlightWait.resetWindow();
	acquire.resetWindow();
	updateUniform.resetWindow();
	record.resetWindow();
	submit.resetWindow();
	renderWait.resetWindow();
	present.resetWindow();
	postprocess.resetWindow();

	for (auto &phase: updateUniformView) {
		phase.resetWindow();
	}
	for (auto &phase: recordView) {
		phase.resetWindow();
	}
	for (auto &phase: submitView) {
		phase.resetWindow();
	}
	for (auto &phase: gpuPass) {
		phase.resetWindow();
	}

	frames			 = 0;
	totalMeshes		 = 0;
	visibleMeshes	 = 0;
	culledMeshes	 = 0;
	skippedMeshes	 = 0;
	drawCalls		 = 0;
	instancedDraws	 = 0;
	pipelineBinds	 = 0;
	descriptorBinds	 = 0;
	syncCalls		 = 0;
	syncSkips		 = 0;
	meshCreations	 = 0;
	meshDestructions = 0;
	uploadWaits		 = 0;
}

bool evan::stats::enabled() noexcept
{
	const int overrideValue = gEnabledOverride.load(std::memory_order_relaxed);
	if (overrideValue >= 0) {
		return overrideValue != 0;
	}
	return statsEnabled();
}

void evan::stats::setEnabledForTesting(std::optional<bool> state) noexcept
{
	if (!state.has_value()) {
		gEnabledOverride.store(-1, std::memory_order_relaxed);
		return;
	}
	gEnabledOverride.store(*state ? 1 : 0, std::memory_order_relaxed);
}

void evan::stats::countSync() noexcept
{
	if (!enabled()) {
		return;
	}
	gSyncCalls.fetch_add(1, std::memory_order_relaxed);
}

void evan::stats::countSyncSkip() noexcept
{
	if (!enabled()) {
		return;
	}
	gSyncSkips.fetch_add(1, std::memory_order_relaxed);
}

void evan::stats::countMeshCreation() noexcept
{
	if (!enabled()) {
		return;
	}
	gMeshCreations.fetch_add(1, std::memory_order_relaxed);
}

void evan::stats::countMeshDestruction() noexcept
{
	if (!enabled()) {
		return;
	}
	gMeshDestructions.fetch_add(1, std::memory_order_relaxed);
}

void evan::stats::countUploadWait() noexcept
{
	if (!enabled()) {
		return;
	}
	gUploadWaits.fetch_add(1, std::memory_order_relaxed);
}

void evan::stats::takeGlobalDeltas(FrameStats &stats) noexcept
{
	const uint64_t syncCalls = gSyncCalls.load(std::memory_order_relaxed);
	const uint64_t syncSkips = gSyncSkips.load(std::memory_order_relaxed);
	const uint64_t meshCreations =
		gMeshCreations.load(std::memory_order_relaxed);
	const uint64_t meshDestructions =
		gMeshDestructions.load(std::memory_order_relaxed);
	const uint64_t uploadWaits = gUploadWaits.load(std::memory_order_relaxed);

	stats.syncCalls += syncCalls - gSnapshot.syncCalls;
	stats.syncSkips += syncSkips - gSnapshot.syncSkips;
	stats.meshCreations += meshCreations - gSnapshot.meshCreations;
	stats.meshDestructions += meshDestructions - gSnapshot.meshDestructions;
	stats.uploadWaits += uploadWaits - gSnapshot.uploadWaits;

	gSnapshot = { syncCalls, syncSkips, meshCreations, meshDestructions,
				  uploadWaits };
}
