#!/usr/bin/env bash
#
# run-poc.sh -- P3 proof-of-concept harness (issue #120, WS4/WS5/WS6 gate).
#
# Runs the macOS GLFW PoC example with EVAN_POC_MESHES=N for each N, collects
# the `evan stats:` report windows emitted by the P2-02 instrumentation and
# prints the averaged fields the trigger table in
# docs/DESIGN_P3_OPTIMIZATIONS.md §4 needs (rec, submit, wait, ubo, pres,
# gpu pass0, draws calls/pso/ds, mesh_churn, uploadwaits).
#
# The example exposes two workload knobs:
#   EVAN_POC_MESHES=N   number of cubes to spawn (default 0 = empty scene)
#   EVAN_POC_FRAMES=M   stop after M frames and exit (default 0 = until close)
#
# Each reported window is a *sum* over the window (except the cpu_ms/gpu_ms
# phases, which are averages), so the script divides the counters by the
# window's `frames=` value before averaging.
#
# Usage:
#   scripts/run-poc.sh [options]
#
# Options:
#   --build-root DIR      CMake build directory (default: <repo>/build)
#   --example-dir DIR     Example binary directory
#                         (default: <build-root>/examples/GLFW-macos-poc)
#   --exe NAME            Executable name (default: GLFW-macos-poc)
#   --mesh-counts "A B"   Object counts to run (default: "100 1000")
#   --interval N          EVAN_STATS_INTERVAL, frames per window (default: 60)
#   --warmup N            Windows to discard as warm-up (default: 2)
#   --windows N           Report windows to target per run (default: 5); it
#                         only sizes the default --frames value
#   --frames N            Total frames per run (default: (warmup + windows)
#                         * interval)
#   --timeout SECONDS     Watchdog for one run (default: 900)
#   --out DIR             Result directory (default: <build-root>/poc-results/
#                         <UTC timestamp>)
#   --window-size LABEL   Window size recorded in the report (default 800x600)
#   --skip-build          Do not build the example first
#   --retries N           Attempts per N when a run dies before producing
#                         enough windows (default: 2)
#   --no-env-info         Do not record machine/GPU information
#   -h, --help            Show this help
#
# Reading the output:
#   <out>/environment.txt   machine, OS and GPU record for the results block
#   <out>/n<N>.log          full application log
#   <out>/n<N>.stats.log    extracted `evan stats:` lines
#   <out>/n<N>.summary.tsv  per-window averages for that N
#   <out>/summary.tsv       one row per N (raw values)
#   <out>/summary.md        the same table as markdown, ready for the design doc
#
# Requires a desktop session (the example creates a Vulkan window); it is not
# meant to run in CI.

set -u
set -o pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

BUILD_ROOT="$REPO_ROOT/build"
EXAMPLE_DIR=""
EXE_NAME="GLFW-macos-poc"
MESH_COUNTS="100 1000"
INTERVAL=60
WARMUP_WINDOWS=2
KEEP_WINDOWS=5
FRAMES=""
TIMEOUT=900
OUT_DIR=""
WINDOW_LABEL="800x600"
SKIP_BUILD=0
ENV_INFO=1
RETRIES=2

usage() {
	awk 'NR > 1 && /^#/ { sub(/^# ?/, ""); print; next } NR > 1 { exit }' "$0"
}

die() {
	echo "error: $*" >&2
	exit 1
}

note() {
	echo "==> $*"
}

require_value() {
	if [ "$2" -lt 2 ]; then
		die "$1 requires a value"
	fi
}

while [ $# -gt 0 ]; do
	case "$1" in
		--build-root)
			require_value "$1" $#
			BUILD_ROOT="$2"
			shift 2
			;;
		--example-dir)
			require_value "$1" $#
			EXAMPLE_DIR="$2"
			shift 2
			;;
		--exe)
			require_value "$1" $#
			EXE_NAME="$2"
			shift 2
			;;
		--mesh-counts)
			require_value "$1" $#
			MESH_COUNTS="$2"
			shift 2
			;;
		--interval)
			require_value "$1" $#
			INTERVAL="$2"
			shift 2
			;;
		--warmup)
			require_value "$1" $#
			WARMUP_WINDOWS="$2"
			shift 2
			;;
		--windows)
			require_value "$1" $#
			KEEP_WINDOWS="$2"
			shift 2
			;;
		--frames)
			require_value "$1" $#
			FRAMES="$2"
			shift 2
			;;
		--timeout)
			require_value "$1" $#
			TIMEOUT="$2"
			shift 2
			;;
		--out)
			require_value "$1" $#
			OUT_DIR="$2"
			shift 2
			;;
		--window-size)
			require_value "$1" $#
			WINDOW_LABEL="$2"
			shift 2
			;;
		--skip-build)
			SKIP_BUILD=1
			shift
			;;
		--retries)
			require_value "$1" $#
			RETRIES="$2"
			shift 2
			;;
		--no-env-info)
			ENV_INFO=0
			shift
			;;
		-h|--help)
			usage
			exit 0
			;;
		*)
			die "unknown option '$1' (try --help)"
			;;
	esac
done

# Every numeric option has to be a plain non-negative integer: the values are
# forwarded to the application and to shell arithmetic.
for numeric in "$INTERVAL" "$WARMUP_WINDOWS" "$KEEP_WINDOWS" "$TIMEOUT" "$RETRIES"; do
	case "$numeric" in
		''|*[!0-9]*)
			die "expected a non-negative integer, got '$numeric'"
			;;
	esac
done

if [ "$INTERVAL" -lt 1 ]; then
	die "--interval must be at least 1"
fi
if [ "$KEEP_WINDOWS" -lt 3 ]; then
	die "--windows must be at least 3 (the PoC needs 3-5 report windows)"
fi
if [ "$RETRIES" -gt 10 ]; then
	die "--retries is capped at 10"
fi
if [ -z "$FRAMES" ]; then
	FRAMES=$(( (WARMUP_WINDOWS + KEEP_WINDOWS) * INTERVAL ))
fi
case "$FRAMES" in
	''|*[!0-9]*)
		die "expected a non-negative integer for --frames, got '$FRAMES'"
		;;
esac
for count in $MESH_COUNTS; do
	case "$count" in
		''|*[!0-9]*)
			die "expected integer mesh counts, got '$MESH_COUNTS'"
			;;
	esac
done

if [ -z "$EXAMPLE_DIR" ]; then
	EXAMPLE_DIR="$BUILD_ROOT/examples/GLFW-macos-poc"
fi
if [ -z "$OUT_DIR" ]; then
	OUT_DIR="$BUILD_ROOT/poc-results/$(date -u +%Y%m%dT%H%M%SZ)"
fi

mkdir -p "$OUT_DIR" || die "cannot create $OUT_DIR"

if [ "$SKIP_BUILD" -eq 0 ]; then
	note "Building $EXE_NAME in $BUILD_ROOT (this also refreshes the copied shaders)"
	cmake --build "$BUILD_ROOT" --target "$EXE_NAME" -j \
		|| die "build failed"
fi

EXE_PATH="$EXAMPLE_DIR/$EXE_NAME"
if [ ! -x "$EXE_PATH" ]; then
	die "$EXE_PATH not found or not executable (build with BUILD_EXAMPLES=ON)"
fi
# Fail fast instead of waiting for the watchdog when the binary predates the
# workload knobs: string literals survive in the executable.
if ! grep -q "EVAN_POC_FRAMES" "$EXE_PATH"; then
	die "$EXE_PATH predates the EVAN_POC_* knobs; rebuild without --skip-build"
fi

if [ "$ENV_INFO" -eq 1 ]; then
	{
		echo "date: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
		echo "host: $(uname -sr) / $(uname -m)"
		if command -v sw_vers >/dev/null 2>&1; then
			echo "macos: $(sw_vers -productVersion) ($(sw_vers -buildVersion))"
		fi
		if command -v sysctl >/dev/null 2>&1; then
			cpu="$(sysctl -n machdep.cpu.brand_string 2>/dev/null)"
			[ -n "$cpu" ] && echo "cpu: $cpu"
		fi
		if command -v system_profiler >/dev/null 2>&1; then
			system_profiler SPDisplaysDataType 2>/dev/null \
				| grep -E "Chipset Model|Vendor|Metal|VRAM" \
				| sed 's/^[[:space:]]*//'
		fi
		if [ -n "${VK_ICD_FILENAMES:-}" ]; then
			echo "vk_icd: $VK_ICD_FILENAMES"
		fi
	} > "$OUT_DIR/environment.txt" 2>&1
fi

SUMMARY_TSV="$OUT_DIR/summary.tsv"
printf 'n\twindows\tframes\tflush\tpre\tacq\twait\tubo\trec\tsubmit\trenderwait\tpres\tpost\tgpu_pass0\ttotal\tvis\tculled\tskip\tdraw_calls\tinstanced\tpso\tds\tsync_calls\tsync_skip\tchurn_new\tchurn_gone\tuploadwaits\tdropped\n' > "$SUMMARY_TSV"

FAILURES=0
RUNS=0

# Runs one N, writes its summary row to $OUT_DIR/n<N>.summary.tsv and returns
# 0 when at least one steady-state window was captured.
run_single() {
	local mesh_count="$1"
	local log_file="$OUT_DIR/n$mesh_count.log"
	local stats_file="$OUT_DIR/n$mesh_count.stats.log"
	local app_pid watchdog_pid app_status window_count

	note "N=$mesh_count: $FRAMES frames, one window every $INTERVAL frames"
	# Run from the example directory: the engine loads shaders/ and textures/
	# relative to the working directory. UTILITY_LOG_LEVEL=info keeps the log
	# to the report windows (debug builds otherwise log per vertex) and
	# NO_COLOR keeps the parsed values free of ANSI escapes.
	(
		cd "$EXAMPLE_DIR" || exit 1
		exec env \
			EVAN_STATS=1 \
			EVAN_STATS_INTERVAL="$INTERVAL" \
			EVAN_POC_MESHES="$mesh_count" \
			EVAN_POC_FRAMES="$FRAMES" \
			UTILITY_LOG_LEVEL=info \
			NO_COLOR=1 \
			"./$EXE_NAME"
	) > "$log_file" 2>&1 &

	app_pid=$!

	# Watchdog: a binary without the EVAN_POC_FRAMES knob runs until the
	# window is closed, so never let a run hang forever.
	(
		elapsed=0
		while [ "$elapsed" -lt "$TIMEOUT" ]; do
			sleep 1
			kill -0 "$app_pid" 2>/dev/null || exit 0
			elapsed=$(( elapsed + 1 ))
		done
		kill -TERM "$app_pid" 2>/dev/null
	) &
	watchdog_pid=$!

	if wait "$app_pid"; then
		app_status=0
	else
		app_status=$?
	fi
	kill -TERM "$watchdog_pid" 2>/dev/null || true
	wait "$watchdog_pid" 2>/dev/null || true

	if ! grep -q "PoC workload:" "$log_file"; then
		echo "  the binary ignored EVAN_POC_MESHES (stale build?)" >&2
		return 1
	fi
	if [ "$app_status" -ne 0 ]; then
		# The report windows may already be complete; retries cover the rest.
		echo "  note: application exited with status $app_status after the run" >&2
	fi

	grep "evan stats:" "$log_file" > "$stats_file" || true
	window_count="$(wc -l < "$stats_file" | tr -d ' ')"

	if [ "$window_count" -le "$WARMUP_WINDOWS" ]; then
		echo "  only $window_count report window(s) captured, need more than" \
			"$WARMUP_WINDOWS (raise --frames or --timeout)" >&2
		return 1
	fi

	# Parse the `key=value` fields of each window and average the windows
	# that follow the warm-up. Phase timings are per-frame averages already;
	# counters are window sums and are divided by the window's `frames=`.
	awk -v warmup="$WARMUP_WINDOWS" -v n="$mesh_count" '
		function parse(line,    parts, nparts, i, p, toks, ntoks, j, t) {
			nparts = split(line, parts, "|")
			for (i = 1; i <= nparts; i++) {
				p = parts[i]
				gsub(/^[ \t]+/, "", p)
				gsub(/[ \t]+$/, "", p)
				if (p == "")
					continue
				# A section label is the text before the first ':' when that
				# comes before the first '=' (e.g. "draws: calls=1").
				label = ""
				colon = index(p, ":")
				equals = index(p, "=")
				if (colon > 0 && (equals == 0 || colon < equals)) {
					label = substr(p, 1, colon - 1)
					p = substr(p, colon + 1)
				}
				ntoks = split(p, toks, /[ \t]+/)
				for (j = 1; j <= ntoks; j++) {
					t = toks[j]
					if (t == "")
						continue
					if (label == "mesh_churn") {
						churn = t
						sub(/^\+/, "", churn)
						split(churn, halves, "/")
						cur["churn_new"] = halves[1] + 0
						sub(/^-/, "", halves[2])
						cur["churn_gone"] = halves[2] + 0
						continue
					}
					eq = index(t, "=")
					if (eq == 0)
						continue
					key = substr(t, 1, eq - 1)
					val = substr(t, eq + 1)
					if (label != "")
						key = label "." key
					cur[key] = val
				}
			}
		}

		BEGIN {
			FS = "|"
			n_avg = split("flush pre acq wait ubo rec submit renderwait pres post gpu_pass0", avg, " ")
			n_cnt = split("total vis culled skip draw_calls instanced pso ds sync_calls sync_skip churn_new churn_gone uploadwaits", cnt, " ")

			map["flush"]       = "cpu_ms(avg).flush"
			map["pre"]         = "cpu_ms(avg).pre"
			map["acq"]         = "cpu_ms(avg).acq"
			map["wait"]        = "cpu_ms(avg).wait"
			map["ubo"]         = "cpu_ms(avg).ubo"
			map["rec"]         = "cpu_ms(avg).rec"
			map["submit"]      = "cpu_ms(avg).submit"
			map["renderwait"]  = "cpu_ms(avg).renderwait"
			map["pres"]        = "cpu_ms(avg).pres"
			map["post"]        = "cpu_ms(avg).post"
			map["gpu_pass0"]   = "gpu_ms(avg).pass0"
			map["total"]       = "meshes.total"
			map["vis"]         = "meshes.vis"
			map["culled"]      = "meshes.culled"
			map["skip"]        = "meshes.skip"
			map["draw_calls"]  = "draws.calls"
			map["instanced"]   = "draws.inst"
			map["pso"]         = "draws.pso"
			map["ds"]          = "draws.ds"
			map["sync_calls"]  = "sync.calls"
			map["sync_skip"]   = "sync.skip"
			map["churn_new"]   = "churn_new"
			map["churn_gone"]  = "churn_gone"
			map["uploadwaits"] = "uploadwaits"
		}

		/evan stats:/ {
			window++
			split("", cur)
			# Drop the log prefix (its timestamp also contains ':') and keep the
			# `evan stats:` label so the first section parses like the others.
			payload = $0
			sub(/^.*evan stats:[ \t]*/, "", payload)
			parse("evan stats: " payload)
			if (window <= warmup)
				next
			frames = cur["evan stats.frames"] + 0
			if (frames <= 0)
				next
			# A window with mesh churn is not steady state: either the startup
			# allocations or a late object registration (the engine registers a
			# debug ray on the first mouse-motion event) still changes the scene.
			if ((cur["churn_new"] + 0) != 0 || (cur["churn_gone"] + 0) != 0) {
				dropped++
				next
			}
			kept++
			kept_frames += frames
			for (i = 1; i <= n_avg; i++) {
				value = cur[map[avg[i]]]
				if (value == "" || value == "--")
					continue
				avg_sum[avg[i]] += value + 0
				avg_count[avg[i]]++
			}
			for (i = 1; i <= n_cnt; i++) {
				value = cur[map[cnt[i]]]
				if (value == "")
					continue
				cnt_sum[cnt[i]] += (value + 0) / frames
				cnt_count[cnt[i]]++
			}
		}

		END {
			if (kept == 0) {
				print "warning: no report window left after the warm-up" > "/dev/stderr"
				exit 1
			}
			line = n "\t" kept "\t" kept_frames
			for (i = 1; i <= n_avg; i++) {
				m = avg[i]
				line = line "\t" (avg_count[m] ? sprintf("%.4f", avg_sum[m] / avg_count[m]) : "n/a")
			}
			for (i = 1; i <= n_cnt; i++) {
				m = cnt[i]
				line = line "\t" (cnt_count[m] ? sprintf("%.4f", cnt_sum[m] / cnt_count[m]) : "n/a")
			}
			print line "\t" dropped
		}
	' "$stats_file" > "$OUT_DIR/n$mesh_count.summary.tsv" || true

	if [ ! -s "$OUT_DIR/n$mesh_count.summary.tsv" ]; then
		echo "  no usable report window for N=$mesh_count" >&2
		return 1
	fi

	return 0
}

for mesh_count in $MESH_COUNTS; do
	RUNS=$(( RUNS + 1 ))
	attempt=0
	succeeded=0

	while [ "$attempt" -le "$RETRIES" ]; do
		attempt=$(( attempt + 1 ))
		if run_single "$mesh_count"; then
			succeeded=1
			break
		fi
		if [ "$attempt" -le "$RETRIES" ]; then
			echo "  retrying N=$mesh_count (attempt $(( attempt + 1 )) of" \
				"$(( RETRIES + 1 )))" >&2
		fi
	done

	if [ "$succeeded" -ne 1 ]; then
		FAILURES=$(( FAILURES + 1 ))
		continue
	fi

	# Drop the N column when appending: the summary header starts with it and
	# each row is appended after its own N.
	tail -n 1 "$OUT_DIR/n$mesh_count.summary.tsv" >> "$SUMMARY_TSV"
done

if [ "$FAILURES" -ne 0 ]; then
	echo "" >&2
	echo "$FAILURES of $RUNS run(s) failed; see $OUT_DIR" >&2
	exit 1
fi

# Render the collected rows twice: a clean markdown table for the design doc
# (summary.md) and per-N decision hints on the console.
{
	echo "### PoC results"
	echo ""
	if [ -s "$OUT_DIR/environment.txt" ]; then
		sed 's/^/> /' "$OUT_DIR/environment.txt"
		echo ""
	fi
	awk -F'\t' '
		NR == 1 {
			print "| N | windows | frames/window | rec (ms) | submit (ms) | wait (ms) | ubo (ms) | pres (ms) | gpu pass0 (ms) | draws calls | pso | ds | mesh_churn (per frame) | uploadwaits | vis | culled |"
			print "| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |"
			next
		}
		{
			gpu = $14
			churn = "n/a"
			if ($25 != "n/a" && $26 != "n/a")
				churn = sprintf("+%.1f/-%.1f", $25, $26)
			printf "| %s | %s | %.0f | %.2f | %.2f | %.2f | %.2f | %.2f | ",
				$1, $2, $3 / $2, $9, $10, $7, $8, $12
			if (gpu == "n/a")
				printf "n/a | "
			else
				printf "%.2f | ", gpu
			printf "%.0f | %.0f | %.0f | %s | %.0f | %.0f | %.0f |\n",
				$19, $21, $22, churn, $27, $16, $17
		}
	' "$SUMMARY_TSV"
	echo ""
	echo '`draws calls` is the measured draw count. The engine also registers a'
	echo 'debug ray object on the first mouse-motion event, so N or N+1 are both'
	echo 'expected depending on whether the pointer entered the window.'

} > "$OUT_DIR/summary.md"

cat "$OUT_DIR/summary.md"

# Decision hints against the §4 trigger table.
awk -F'\t' -v window_label="$WINDOW_LABEL" -v interval="$INTERVAL" '
	NR == 1 { next }
	{
		flush = $4; pre = $5; acq = $6; wait = $7; ubo = $8; rec = $9
		submit = $10; renderwait = $11; pres = $12; post = $13
		calls = $19; ds = $22; uploadwaits = $27; dropped = $28; windows = $2

		measured = flush + pre + acq + wait + ubo + rec + submit + pres + post
		if (renderwait != "n/a")
			measured += renderwait
		rec_share = measured > 0 ? 100 * rec / measured : 0
		wait_share = measured > 0 ? 100 * wait / measured : 0
		if (renderwait != "n/a" && measured > 0)
			wait_share = 100 * (wait + renderwait) / measured

		printf "N=%s (window %s, %s frames per report window):\n", $1, window_label, interval
		printf "  rec share of measured CPU frame: %.1f %% (WS4 trigger is per-view/XR stereo: > ~15 %%)\n", rec_share
		printf "  wait+renderwait share:           %.1f %% (WS5 trigger: these dominate the frame)\n", wait_share
		printf "  descriptor binds per frame:      %.0f (WS6 trigger: grows with material count)\n", ds

		if (calls <= 0)
			printf "  warning: no draw calls — the workload did not render (check shaders/)\n"
		if (uploadwaits != "n/a" && uploadwaits != 0)
			printf "  warning: blocking upload waits present (expected 0 in steady state)\n"
		if (dropped > 0)
			printf "  note: excluded %.0f window(s) with mesh churn (scene still changing)\n", dropped
		if (windows < 3)
			printf "  note: only %.0f steady-state window(s) — raise --frames for more\n", windows
		print ""
	}
' "$SUMMARY_TSV"

if [ "$ENV_INFO" -eq 1 ] && [ -s "$OUT_DIR/environment.txt" ]; then
	note "Environment record ($OUT_DIR/environment.txt)"
fi

note "Results in $OUT_DIR"
