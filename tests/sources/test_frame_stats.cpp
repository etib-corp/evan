/*
 Copyright (c) 2026 ETIB Corporation

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

#include <evan/FrameStats.hpp>

#include <optional>

#include <gtest/gtest.h>

namespace xider::tests
{
	/**
	 * @brief Phase samples accumulate, average and survive a window reset
	 * with their most recent value readable.
	 */
	TEST(FrameStatsTest, PhaseStatsAccumulateAverageAndReset)
	{
		evan::PhaseStats phase;

		EXPECT_EQ(phase.averageNs(), 0u);

		phase.add(100);
		phase.add(200);

		EXPECT_EQ(phase.lastNs, 200u);
		EXPECT_EQ(phase.samples, 2u);
		EXPECT_EQ(phase.averageNs(), 150u);

		phase.resetWindow();

		EXPECT_EQ(phase.samples, 0u);
		EXPECT_EQ(phase.averageNs(), 0u);
		EXPECT_EQ(phase.lastNs, 200u);
	}

	/**
	 * @brief The scoped timer records only when its FrameStats is enabled.
	 */
	TEST(FrameStatsTest, ScopedTimerFollowsEnabledFlag)
	{
		evan::FrameStats stats;

		stats.enabled = true;
		{
			evan::ScopedTimer timer = stats.timer(stats.record);
		}
		EXPECT_EQ(stats.record.samples, 1u);

		stats.enabled = false;
		{
			evan::ScopedTimer timer = stats.timer(stats.record);
		}
		EXPECT_EQ(stats.record.samples, 1u);
	}

	/**
	 * @brief A window reset clears samples and counters but keeps the
	 * enabled/available flags.
	 */
	TEST(FrameStatsTest, ResetWindowKeepsAvailabilityFlags)
	{
		evan::FrameStats stats;
		stats.enabled	   = true;
		stats.gpuAvailable = true;
		stats.frames	   = 7;
		stats.drawCalls	   = 42;
		stats.record.add(1234);

		stats.resetWindow();

		EXPECT_TRUE(stats.enabled);
		EXPECT_TRUE(stats.gpuAvailable);
		EXPECT_EQ(stats.frames, 0u);
		EXPECT_EQ(stats.drawCalls, 0u);
		EXPECT_EQ(stats.record.samples, 0u);
	}

	/**
	 * @brief EVAN_STATS-style flag parsing: empty and "0" disable, anything
	 * else enables.
	 */
	TEST(FrameStatsTest, FlagParsing)
	{
		EXPECT_FALSE(::statsFlagEnabled(nullptr));
		EXPECT_FALSE(::statsFlagEnabled(""));
		EXPECT_FALSE(::statsFlagEnabled("0"));
		EXPECT_TRUE(::statsFlagEnabled("1"));
		EXPECT_TRUE(::statsFlagEnabled("on"));
		EXPECT_TRUE(::statsFlagEnabled("true"));
	}

	/**
	 * @brief Interval parsing falls back to the default window for missing,
	 * zero, negative, non-numeric or absurd values.
	 */
	TEST(FrameStatsTest, IntervalParsing)
	{
		EXPECT_EQ(::statsIntervalFromValue(nullptr), 120u);
		EXPECT_EQ(::statsIntervalFromValue(""), 120u);
		EXPECT_EQ(::statsIntervalFromValue("0"), 120u);
		EXPECT_EQ(::statsIntervalFromValue("-3"), 120u);
		EXPECT_EQ(::statsIntervalFromValue("abc"), 120u);
		EXPECT_EQ(::statsIntervalFromValue("60"), 60u);
		EXPECT_EQ(::statsIntervalFromValue("12x"), 12u);
	}

	/**
	 * @brief The global counters are folded into a window exactly once, and
	 * are gated by the (overridable) enabled flag.
	 */
	TEST(FrameStatsTest, GlobalCountersReportDeltasOnce)
	{
		evan::stats::setEnabledForTesting(true);

		// Drop whatever earlier tests accumulated.
		evan::FrameStats stats;
		evan::stats::takeGlobalDeltas(stats);

		evan::stats::countSync();
		evan::stats::countSync();
		evan::stats::countSyncSkip();
		evan::stats::countMeshCreation();
		evan::stats::countMeshDestruction();
		evan::stats::countUploadWait();

		evan::stats::takeGlobalDeltas(stats);

		EXPECT_EQ(stats.syncCalls, 2u);
		EXPECT_EQ(stats.syncSkips, 1u);
		EXPECT_EQ(stats.meshCreations, 1u);
		EXPECT_EQ(stats.meshDestructions, 1u);
		EXPECT_EQ(stats.uploadWaits, 1u);

		// A second take without new increments adds nothing.
		evan::stats::takeGlobalDeltas(stats);
		EXPECT_EQ(stats.syncCalls, 2u);

		// Forced off: increments are dropped.
		evan::stats::setEnabledForTesting(false);
		evan::stats::countSync();
		evan::stats::takeGlobalDeltas(stats);
		EXPECT_EQ(stats.syncCalls, 2u);

		// Restore the environment-driven gate for the rest of the binary.
		evan::stats::setEnabledForTesting(std::nullopt);
	}

	/**
	 * @brief Narrow timestamp counters only expose their low valid bits.
	 */
	TEST(FrameStatsTest, TimestampMasking)
	{
		EXPECT_EQ(evan::stats::maskTimestamp(0x1'2345'6789ull, 64),
				  0x1'2345'6789ull);
		EXPECT_EQ(evan::stats::maskTimestamp(0x1'2345'6789ull, 32),
				  0x2345'6789ull);
		EXPECT_EQ(evan::stats::maskTimestamp(123, 0), 0u);
	}

	/**
	 * @brief Timestamp deltas convert through the tick period, and unusable
	 * samples (counter wrap, unknown period) are rejected.
	 */
	TEST(FrameStatsTest, TimestampDeltaConversion)
	{
		uint64_t deltaNs = 0;

		// 50 ticks at 2 ns per tick.
		EXPECT_TRUE(evan::stats::timestampDeltaNs(100, 150, 2.0f, deltaNs));
		EXPECT_EQ(deltaNs, 100u);

		// Fractional period: 4 ticks at 0.25 ns per tick rounds down.
		EXPECT_TRUE(evan::stats::timestampDeltaNs(10, 14, 0.25f, deltaNs));
		EXPECT_EQ(deltaNs, 1u);

		// Counter went backwards: unusable sample.
		EXPECT_FALSE(evan::stats::timestampDeltaNs(150, 100, 1.0f, deltaNs));

		// Unknown period: unusable sample.
		EXPECT_FALSE(evan::stats::timestampDeltaNs(0, 100, 0.0f, deltaNs));
	}
}	 // namespace xider::tests
