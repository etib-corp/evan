#!/usr/bin/env bash
#
# run-stress.sh -- find the limit of the Evan renderer (issue #173).
#
# Drives the GLFW-stress example to answer, for every frame-rate threshold:
#
#   * how many meshes (draw calls) the engine holds the threshold with,
#   * how many distinct pipelines it holds the threshold with,
#
# and records the rendering indicators of each boundary configuration (draw
# calls, pipeline/descriptor binds, culling, CPU phases, GPU pass time).
#
# The example measures one configuration and prints an `evan stress:` record;
# this script runs it in an exponential + binary search, caches every probed
# (meshes, pipelines) pair and renders the results as markdown and CSV.
#
# Usage:
#   scripts/run-stress.sh [options]
#
# Options:
#   --thresholds "T ..."    Frame-rate thresholds to solve (default
#                           "120 100 75 60 30 24")
#   --meshes-max N          Upper bound of the mesh search (default 131072)
#   --pipelines-max K       Upper bound of the pipeline search (default 32)
#   --base-meshes N         Scene size while sweeping pipelines (default 1000)
#   --base-pipelines K      Pipeline count while sweeping meshes (default 1)
#   --frames N              Measured frames per probe (default 240)
#   --warmup N              Warm-up frames per probe (default 60)
#   --refine N              Binary-search steps per boundary (default 4)
#   --max-probes N          Hard cap on probes for the whole run (default 400)
#   --present-mode M        fifo|mailbox|immediate|auto (default mailbox)
#   --instancing on|off     Merge identical draws (default off)
#   --indirect on|off       Indirect draws (default off)
#   --culling on|off        Frustum culling (default on)
#   --window WxH            Window size (default 800x600)
#   --build-root DIR        CMake build directory (default <repo>/build)
#   --example-dir DIR       Example binary directory
#                           (default <build-root>/examples/GLFW-stress)
#   --exe NAME              Executable name (default GLFW-stress)
#   --out DIR               Result directory (default <build-root>/stress-
#                           results/<UTC timestamp>)
#   --timeout SECONDS       Watchdog for one probe (default 300)
#   --retries N             Attempts per probe (default 1)
#   --quick                 Small smoke run: 2048 meshes max, 8 pipelines,
#                           90 frames, thresholds "60 30"
#   --no-env-info           Do not record machine/GPU information
#   --skip-build            Do not build the example first
#   -h, --help              Show this help
#
# Reading the output:
#   <out>/environment.txt   machine, OS and GPU record
#   <out>/probe-*.log       full log of one probe
#   <out>/runs.tsv          every probed configuration with its indicators
#   <out>/results.csv       the example's own CSV rows (raw frame times summary)
#   <out>/summary.md        the limit tables, ready for a results document
#
# The measured frame rate is only meaningful above the display refresh rate if
# the present mode is uncapped, which is why --present-mode defaults to
# mailbox. With fifo, every threshold above the refresh rate is unreachable.
#
# Requires a desktop session (the example creates a Vulkan window); it is not
# meant to run in CI.

set -u
set -o pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

BUILD_ROOT="$REPO_ROOT/build"
EXAMPLE_DIR=""
EXE_NAME="GLFW-stress"
THRESHOLDS="120 100 75 60 30 24"
MESHES_MAX=131072
PIPELINES_MAX=32
BASE_MESHES=1000
BASE_PIPELINES=1
FRAMES=240
WARMUP=60
REFINE=4
MAX_PROBES=400
PRESENT_MODE="mailbox"
INSTANCING="off"
INDIRECT="off"
CULLING="on"
WINDOW_LABEL="800x600"
OUT_DIR=""
TIMEOUT=300
RETRIES=1
ENV_INFO=1
SKIP_BUILD=0

PROBES_DONE=0
LIMIT=0
LIMIT_FPS=0
CURRENT_APP_PID=""

# Kills the probe still running when the script is interrupted, so no window
# and no Vulkan process outlives a Ctrl-C.
cleanup() {
	if [ -n "$CURRENT_APP_PID" ]; then
		kill -TERM "$CURRENT_APP_PID" 2>/dev/null || true
		CURRENT_APP_PID=""
	fi
}
trap cleanup EXIT INT TERM

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
		--thresholds) require_value "$1" $#; THRESHOLDS="$2"; shift 2 ;;
		--meshes-max) require_value "$1" $#; MESHES_MAX="$2"; shift 2 ;;
		--pipelines-max) require_value "$1" $#; PIPELINES_MAX="$2"; shift 2 ;;
		--base-meshes) require_value "$1" $#; BASE_MESHES="$2"; shift 2 ;;
		--base-pipelines) require_value "$1" $#; BASE_PIPELINES="$2"; shift 2 ;;
		--frames) require_value "$1" $#; FRAMES="$2"; shift 2 ;;
		--warmup) require_value "$1" $#; WARMUP="$2"; shift 2 ;;
		--refine) require_value "$1" $#; REFINE="$2"; shift 2 ;;
		--max-probes) require_value "$1" $#; MAX_PROBES="$2"; shift 2 ;;
		--present-mode) require_value "$1" $#; PRESENT_MODE="$2"; shift 2 ;;
		--instancing) require_value "$1" $#; INSTANCING="$2"; shift 2 ;;
		--indirect) require_value "$1" $#; INDIRECT="$2"; shift 2 ;;
		--culling) require_value "$1" $#; CULLING="$2"; shift 2 ;;
		--window) require_value "$1" $#; WINDOW_LABEL="$2"; shift 2 ;;
		--build-root) require_value "$1" $#; BUILD_ROOT="$2"; shift 2 ;;
		--example-dir) require_value "$1" $#; EXAMPLE_DIR="$2"; shift 2 ;;
		--exe) require_value "$1" $#; EXE_NAME="$2"; shift 2 ;;
		--out) require_value "$1" $#; OUT_DIR="$2"; shift 2 ;;
		--timeout) require_value "$1" $#; TIMEOUT="$2"; shift 2 ;;
		--retries) require_value "$1" $#; RETRIES="$2"; shift 2 ;;
		--quick)
			MESHES_MAX=2048
			PIPELINES_MAX=8
			FRAMES=90
			WARMUP=20
			MAX_PROBES=60
			THRESHOLDS="60 30"
			shift
			;;
		--no-env-info) ENV_INFO=0; shift ;;
		--skip-build) SKIP_BUILD=1; shift ;;
		-h|--help) usage; exit 0 ;;
		*) die "unknown option '$1' (try --help)" ;;
	esac
done

for numeric in "$MESHES_MAX" "$PIPELINES_MAX" "$BASE_MESHES" "$BASE_PIPELINES" \
	"$FRAMES" "$WARMUP" "$REFINE" "$MAX_PROBES" "$TIMEOUT" "$RETRIES"; do
	case "$numeric" in
		''|*[!0-9]*) die "expected a non-negative integer, got '$numeric'" ;;
	esac
done
for threshold in $THRESHOLDS; do
	case "$threshold" in
		''|*[!0-9]*) die "expected integer thresholds, got '$THRESHOLDS'" ;;
	esac
	[ "$threshold" -ge 1 ] || die "thresholds must be at least 1 fps"
done

[ "$MESHES_MAX" -ge 1 ] || die "--meshes-max must be at least 1"
[ "$PIPELINES_MAX" -ge 1 ] || die "--pipelines-max must be at least 1"
[ "$BASE_MESHES" -ge 1 ] || die "--base-meshes must be at least 1"
[ "$BASE_PIPELINES" -ge 1 ] || die "--base-pipelines must be at least 1"
[ "$FRAMES" -ge 30 ] || die "--frames must be at least 30"
[ "$MAX_PROBES" -ge 1 ] || die "--max-probes must be at least 1"

for switch in "$INSTANCING" "$INDIRECT" "$CULLING"; do
	case "$switch" in
		on|off|1|0|true|false|yes|no) ;;
		*) die "expected on/off, got '$switch'" ;;
	esac
done
case "$PRESENT_MODE" in
	fifo|mailbox|immediate|auto|vsync|nowait|0) ;;
	*) die "expected fifo|mailbox|immediate|auto, got '$PRESENT_MODE'" ;;
esac

if [ -z "$EXAMPLE_DIR" ]; then
	EXAMPLE_DIR="$BUILD_ROOT/examples/$EXE_NAME"
fi
if [ -z "$OUT_DIR" ]; then
	OUT_DIR="$BUILD_ROOT/stress-results/$(date -u +%Y%m%dT%H%M%SZ)"
fi

mkdir -p "$OUT_DIR" || die "cannot create $OUT_DIR"

if [ "$SKIP_BUILD" -eq 0 ]; then
	note "Building $EXE_NAME in $BUILD_ROOT"
	cmake --build "$BUILD_ROOT" --target "$EXE_NAME" -j \
		|| die "build failed (configure with -DBUILD_EXAMPLES=ON)"
fi

EXE_PATH="$EXAMPLE_DIR/$EXE_NAME"
if [ ! -x "$EXE_PATH" ]; then
	die "$EXE_PATH not found or not executable (build with BUILD_EXAMPLES=ON)"
fi
if ! grep -q "evan stress:" "$EXE_PATH"; then
	die "$EXE_PATH predates the stress harness; rebuild without --skip-build"
fi

CSV_PATH="$OUT_DIR/results.csv"
RUNS_TSV="$OUT_DIR/runs.tsv"

# Column 3 is the decision metric: the frame rate of the median frame.
# A single driver-side stall (first use of a pipeline) must not decide a
# boundary, so the mean frame rate is kept as column 4 and reported next to it.
printf 'meshes\tpipelines\tfps_p50\tfps\tfps_1pct_low\tp50\tp95\tp99\tmax\tinit_ms\tscene_ms\tpresent\tinstancing\tindirect\tculling\tdraws\tinstanced\tpso\tds\tvis\tculled\tskip\trec\tsubmit\twait\tubo\tpres\tgpu\tupload_waits\tcreated\n' > "$RUNS_TSV"

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
		echo "window: $WINDOW_LABEL"
		echo "present_mode: $PRESENT_MODE"
		echo "frames_per_probe: $FRAMES (warmup $WARMUP)"
	} > "$OUT_DIR/environment.txt" 2>&1
fi

# Prints the value of one key of an `evan stress:` record.
record_value() {
	printf '%s\n' "$1" | tr ' ' '\n' | sed -n "s/^$2=//p" | head -n 1
}

# Returns 0 when the measured fps meets the threshold.
meets_threshold() {
	awk -v fps="$1" -v threshold="$2" 'BEGIN { exit !(fps + 0 >= threshold + 0) }'
}

# Looks a configuration up in runs.tsv; echoes its fps, fails when absent.
cached_fps() {
	awk -F'\t' -v m="$1" -v p="$2" '
		NR > 1 && $1 == m && $2 == p { found = $3 }
		END { if (found != "") print found; else exit 1 }
	' "$RUNS_TSV"
}

# Runs one probe and appends its row to runs.tsv. Sets PROBE_FPS.
probe() {
	local meshes="$1"
	local pipelines="$2"

	local log_file="$OUT_DIR/probe-m${meshes}-p${pipelines}.log"
	local attempt=0
	local fps=""

	if [ "$PROBES_DONE" -ge "$MAX_PROBES" ]; then
		return 1
	fi

	if fps="$(cached_fps "$meshes" "$pipelines")"; then
		PROBE_FPS="$fps"
		return 0
	fi

	while [ "$attempt" -le "$RETRIES" ]; do
		attempt=$((attempt + 1))
		PROBES_DONE=$((PROBES_DONE + 1))

		(
			cd "$EXAMPLE_DIR" || exit 1
			exec env \
				NO_COLOR=1 \
				UTILITY_LOG_LEVEL=warning \
				"$EXE_PATH" \
				--quiet \
				--meshes "$meshes" \
				--pipelines "$pipelines" \
				--frames "$FRAMES" \
				--warmup "$WARMUP" \
				--present-mode "$PRESENT_MODE" \
				--instancing "$INSTANCING" \
				--indirect "$INDIRECT" \
				--culling "$CULLING" \
				--window "$WINDOW_LABEL" \
				--csv "$CSV_PATH"
		) > "$log_file" 2>&1 &
		local app_pid=$!
		CURRENT_APP_PID="$app_pid"

		(
			elapsed=0
			while [ "$elapsed" -lt "$TIMEOUT" ]; do
				sleep 1
				kill -0 "$app_pid" 2>/dev/null || exit 0
				elapsed=$((elapsed + 1))
			done
			kill -TERM "$app_pid" 2>/dev/null
		) &
		local watchdog_pid=$!

		wait "$app_pid"
		CURRENT_APP_PID=""
		kill -TERM "$watchdog_pid" 2>/dev/null || true
		wait "$watchdog_pid" 2>/dev/null || true

		local record
		record="$(grep -m 1 'evan stress:' "$log_file" || true)"

		if [ -n "$record" ]; then
			PROBE_FPS="$(record_value "$record" fps_p50)"
			printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
				"$(record_value "$record" meshes)" \
				"$(record_value "$record" pipelines)" \
				"${PROBE_FPS:-0}" \
				"$(record_value "$record" fps)" \
				"$(record_value "$record" fps_1pct_low)" \
				"$(record_value "$record" frame_ms_p50)" \
				"$(record_value "$record" frame_ms_p95)" \
				"$(record_value "$record" frame_ms_p99)" \
				"$(record_value "$record" frame_ms_max)" \
				"$(record_value "$record" init_ms)" \
				"$(record_value "$record" scene_ms)" \
				"$(record_value "$record" present_mode)" \
				"$(record_value "$record" instancing)" \
				"$(record_value "$record" indirect)" \
				"$(record_value "$record" culling)" \
				"$(record_value "$record" draws)" \
				"$(record_value "$record" instanced)" \
				"$(record_value "$record" pso_binds)" \
				"$(record_value "$record" ds_binds)" \
				"$(record_value "$record" vis)" \
				"$(record_value "$record" culled)" \
				"$(record_value "$record" skip)" \
				"$(record_value "$record" rec_ms)" \
				"$(record_value "$record" submit_ms)" \
				"$(record_value "$record" wait_ms)" \
				"$(record_value "$record" ubo_ms)" \
				"$(record_value "$record" pres_ms)" \
				"$(record_value "$record" gpu_ms)" \
				"$(record_value "$record" upload_waits)" \
				"$(record_value "$record" created)" >> "$RUNS_TSV"
			return 0
		fi

		echo "  probe meshes=$meshes pipelines=$pipelines failed" \
			"(attempt $attempt, see $log_file)" >&2
	done

	return 1
}

# Probes one configuration and stores its frame rate in PROBE_FPS.
probe_pair() {  # $1=dimension $2=value
	if [ "$1" = meshes ]; then
		probe "$2" "$BASE_PIPELINES"
	else
		probe "$BASE_MESHES" "$2"
	fi
}

# Finds the largest swept value that still meets @p threshold.
# Sets LIMIT and LIMIT_FPS.
search_limit() {  # $1=dimension $2=threshold
	local dimension="$1"
	local threshold="$2"
	local maximum="$MESHES_MAX"
	local low=0
	local high=""
	local candidate=1

	[ "$dimension" = pipelines ] && maximum="$PIPELINES_MAX"

	LIMIT=0
	LIMIT_FPS=0

	while :; do
		if ! probe_pair "$dimension" "$candidate"; then
			return 1
		fi

		if meets_threshold "$PROBE_FPS" "$threshold"; then
			low="$candidate"
			LIMIT_FPS="$PROBE_FPS"
			[ "$candidate" -ge "$maximum" ] && break
			next=$((candidate * 2))
			[ "$next" -gt "$maximum" ] && next="$maximum"
			[ "$next" -eq "$candidate" ] && break
			candidate="$next"
		else
			high="$candidate"
			break
		fi

		if [ "$PROBES_DONE" -ge "$MAX_PROBES" ]; then
			break
		fi
	done

	local step=0
	while [ -n "$high" ] && [ "$step" -lt "$REFINE" ] \
		&& [ "$PROBES_DONE" -lt "$MAX_PROBES" ]; do
		local tolerance=$((low / 50))
		[ "$tolerance" -lt 1 ] && tolerance=1
		[ $((high - low)) -le "$tolerance" ] && break

		local middle=$((low + (high - low) / 2))
		[ "$middle" -le "$low" ] && break

		if ! probe_pair "$dimension" "$middle"; then
			break
		fi

		if meets_threshold "$PROBE_FPS" "$threshold"; then
			low="$middle"
			LIMIT_FPS="$PROBE_FPS"
		else
			high="$middle"
		fi
		step=$((step + 1))
	done

	LIMIT="$low"
	return 0
}

note "Thresholds: $THRESHOLDS (fps), present mode: $PRESENT_MODE"
note "Mesh sweep: 1..$MESHES_MAX at $BASE_PIPELINES pipeline(s)"
note "Pipeline sweep: 1..$PIPELINES_MAX at $BASE_MESHES mesh(es)"
note "Probe budget: $MAX_PROBES x ($WARMUP warm-up + $FRAMES measured frames)"

LIMITS_TSV="$OUT_DIR/limits.tsv"
printf 'threshold\tmesh_limit\tmesh_fps\tpipe_limit\tpipe_fps\n' > "$LIMITS_TSV"

for threshold in $THRESHOLDS; do
	note "Solving ${threshold} fps: meshes"
	if search_limit meshes "$threshold"; then
		mesh_limit="$LIMIT"
		mesh_fps="$LIMIT_FPS"
		echo "  max meshes >= ${threshold} fps: $mesh_limit (measured $mesh_fps fps)"
	elif [ "$PROBES_DONE" -ge "$MAX_PROBES" ]; then
		mesh_limit="budget"
		mesh_fps="-"
		echo "  probe budget exhausted before ${threshold} fps (meshes)" >&2
	else
		mesh_limit="failed"
		mesh_fps="-"
		echo "  mesh sweep failed at ${threshold} fps" >&2
	fi

	note "Solving ${threshold} fps: pipelines"
	if search_limit pipelines "$threshold"; then
		pipe_limit="$LIMIT"
		pipe_fps="$LIMIT_FPS"
		echo "  max pipelines >= ${threshold} fps: $pipe_limit (measured $pipe_fps fps)"
	elif [ "$PROBES_DONE" -ge "$MAX_PROBES" ]; then
		pipe_limit="budget"
		pipe_fps="-"
		echo "  probe budget exhausted before ${threshold} fps (pipelines)" >&2
	else
		pipe_limit="failed"
		pipe_fps="-"
		echo "  pipeline sweep failed at ${threshold} fps" >&2
	fi

	printf '%s\t%s\t%s\t%s\t%s\n' \
		"$threshold" "$mesh_limit" "$mesh_fps" "$pipe_limit" "$pipe_fps" \
		>> "$LIMITS_TSV"
done

if [ "$(wc -l < "$RUNS_TSV" | tr -d ' ')" -le 1 ]; then
	die "no probe produced a record; see $OUT_DIR"
fi

# Renders the indicators of every boundary configuration. Runs.tsv columns:
# 1 meshes, 2 pipelines, 3 fps_p50, 4 fps, 5 fps_1pct_low, 6 p50, 7 p95,
# 8 p99, 9 max, 10 init_ms, 11 scene_ms, 12 present, 13 instancing,
# 14 indirect, 15 culling, 16 draws, 17 instanced, 18 pso, 19 ds, 20 vis,
# 21 culled, 22 skip, 23 rec, 24 submit, 25 wait, 26 ubo, 27 pres, 28 gpu,
# 29 upload_waits, 30 created.
report_indicators() {  # $1=meshes|pipelines
	local dimension="$1"

	awk -F'\t' \
		-v dimension="$dimension" \
		-v base_meshes="$BASE_MESHES" \
		-v base_pipelines="$BASE_PIPELINES" '
		FILENAME == ARGV[1] {
			if (FNR > 1) {
				order[++count] = $1
				mesh_limit[$1] = $2
				mesh_fps[$1] = $3
				pipe_limit[$1] = $4
				pipe_fps[$1] = $5
			}
			next
		}
		FNR > 1 {
			row[$1 ":" $2] = $0
			next
		}
		END {
			for (i = 1; i <= count; i++) {
				threshold = order[i]

				if (dimension == "meshes") {
					limit = mesh_limit[threshold]
					fps = mesh_fps[threshold]
					key = limit ":" base_pipelines
				} else {
					limit = pipe_limit[threshold]
					fps = pipe_fps[threshold]
					key = base_meshes ":" limit
				}

				if (limit == "failed" || limit == "budget" || limit == ""
					|| !(key in row)) {
					printf "| %s fps | %s | %s | - | - | - | - | - | - | - | - | - |\n",
						threshold, (limit == "" ? "-" : limit), fps
					continue
				}

				split(row[key], f, "\t")
				printf "| %s fps | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
					threshold, limit, fps, f[6], f[7], f[16], f[18], f[19],
					f[20], f[21], f[23], f[28]
			}
		}
	' "$LIMITS_TSV" "$RUNS_TSV"
}

{
	echo "### Frame-rate limits"
	echo ""
	if [ -s "$OUT_DIR/environment.txt" ]; then
		sed 's/^/> /' "$OUT_DIR/environment.txt"
		echo ""
	fi
	echo "| Threshold | Max meshes | FPS at limit (p50) | Max pipelines | FPS at limit (p50) |"
	echo "| --- | --- | --- | --- | --- |"
	awk -F'\t' 'NR > 1 {
		printf "| %s fps | %s | %s | %s | %s |\n", $1, $2, $3, $4, $5
	}' "$LIMITS_TSV"
	echo ""
	echo "Mesh sweep: \`$BASE_PIPELINES\` pipeline(s), $WINDOW_LABEL,"
	echo "\`$FRAMES\` measured frames after \`$WARMUP\` warm-up frames,"
	echo "present mode \`$PRESENT_MODE\`, culling \`$CULLING\`, instancing"
	echo "\`$INSTANCING\`, indirect \`$INDIRECT\`."
	echo "Pipeline sweep: \`$BASE_MESHES\` mesh(es), same frame budget."
	echo ""
	echo "A limit of \`0\` means the threshold is unreachable even with one"
	echo "mesh; a limit equal to the search bound means the bound was hit"
	echo "(\`$MESHES_MAX\` meshes / \`$PIPELINES_MAX\` pipelines)."
	echo "\`budget\` means --max-probes ran out before that threshold was"
	echo "solved; rerun with a larger budget."
	echo ""
	echo "### Rendering indicators at the mesh limit"
	echo ""
	echo "| Threshold | Limit | FPS | p50 (ms) | p95 (ms) | draws | pso | ds | vis | culled | rec (ms) | gpu (ms) |"
	echo "| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |"
	report_indicators meshes
	echo ""
	echo "### Rendering indicators at the pipeline limit"
	echo ""
	echo "| Threshold | Limit | FPS | p50 (ms) | p95 (ms) | draws | pso | ds | vis | culled | rec (ms) | gpu (ms) |"
	echo "| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |"
	report_indicators pipelines
} > "$OUT_DIR/summary.md"

cat "$OUT_DIR/summary.md"

case "$PRESENT_MODE" in
	fifo|vsync|0)
		echo "" >&2
		echo "warning: fifo/vsync caps the frame rate at the display" >&2
		echo "refresh rate, so no threshold above it can be reached; use" >&2
		echo "--present-mode mailbox for a throughput ceiling." >&2
		;;
esac

note "Probes run: $PROBES_DONE (rows in $RUNS_TSV)"
note "Results in $OUT_DIR"
