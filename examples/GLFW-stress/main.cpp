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

/**
 * @file main.cpp
 * @brief One measured stress configuration of the Evan renderer.
 *
 * The executable renders a grid of @c --meshes cubes spread over
 * @c --pipelines distinct graphics pipelines, warms up, measures every frame
 * time, and prints a single machine-readable `evan stress:` record with the
 * frame rate, frame-time percentiles and the renderer's own counters (draw
 * calls, pipeline/descriptor binds, culling, CPU phases, GPU pass time).
 *
 * It measures one configuration; scripts/run-stress.sh drives it in a sweep
 * and finds the frame-rate limits. See docs/STRESS_TESTING.md.
 */

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#if defined(_WIN32)
	#include <process.h>
#else
	#include <unistd.h>
#endif

#include <glm/gtc/matrix_transform.hpp>

#if defined(__APPLE__)
	#include <evan/glfw/platform/MacOsDesktopPlatform.hpp>
#elif defined(__linux__)
	#include <evan/glfw/platform/LinuxDesktopPlatform.hpp>
#elif defined(_WIN32)
	#include <evan/glfw/platform/WindowsPlatform.hpp>
#else
	#error "GLFW-stress has no desktop platform for this operating system"
#endif

#include <evan/Engine.hpp>

#include <utility/graphic/color.hpp>
#include <utility/graphic/mesh.hpp>
#include <utility/graphic/vertex.hpp>

namespace
{
	/**
	 * @brief Largest pipeline count the harness accepts.
	 *
	 * Every pipeline needs a material variant, and every material allocates
	 * descriptor sets from the renderer's fixed-size pool, so an unbounded
	 * count would only turn into an allocation failure. The sweep script caps
	 * well below this.
	 */
	constexpr std::size_t kMaxPipelines = 256;

	/**
	 * @brief Configuration of one measured run.
	 */
	struct Configuration {
		std::size_t meshes	  = 1000;
		std::size_t pipelines = 1;
		std::size_t frames	  = 300;
		std::size_t warmup	  = 60;
		std::size_t width	  = 800;
		std::size_t height	  = 600;
		bool instancing		  = false;
		bool indirect		  = false;
		bool culling		  = true;
		bool verbose		  = true;
		std::string presentMode;
		std::string csvPath;
	};

	/**
	 * @brief Frame-time distribution and frame rate of the measured frames.
	 */
	struct Measurement {
		double fps		  = 0.0;	///< Mean frame rate over the window.
		double fpsP50	  = 0.0;	///< Frame rate of the median frame.
		double fpsLow1pct = 0.0;	///< Frame rate of the slowest frame.
		double p50ms	  = 0.0;
		double p95ms	  = 0.0;
		double p99ms	  = 0.0;
		double maxMs	  = 0.0;
		double elapsedSec = 0.0;
	};

	/**
	 * @brief Returns the current process id, used to name the scratch dir.
	 *
	 * @return The process id.
	 */
	long long currentProcessId()
	{
#if defined(_WIN32)
		return static_cast<long long>(_getpid());
#else
		return static_cast<long long>(::getpid());
#endif
	}

	/**
	 * @brief Sets an environment variable only when it is not already set.
	 *
	 * The harness publishes its knobs to the engine this way (present mode,
	 * culling, statistics) without overriding an explicit user choice.
	 *
	 * @param name Variable name.
	 * @param value Value to set when the variable is missing or empty.
	 */
	void setEnvIfUnset(const char *name, const std::string &value)
	{
		const char *current = std::getenv(name);

		if (current != nullptr && current[0] != '\0') {
			return;
		}
#if defined(_WIN32)
		_putenv_s(name, value.c_str());
#else
		::setenv(name, value.c_str(), 1);
#endif
	}

	/**
	 * @brief Reads an unsigned count from the environment.
	 *
	 * @param name Variable name.
	 * @param fallback Value used when the variable is absent or invalid.
	 * @return The parsed count, or @p fallback.
	 */
	std::size_t readCountEnv(const char *name, std::size_t fallback)
	{
		const char *value = std::getenv(name);

		if (value == nullptr || value[0] == '\0') {
			return fallback;
		}

		try {
			const long long parsed = std::stoll(value);

			if (parsed < 0) {
				return fallback;
			}
			return static_cast<std::size_t>(parsed);
		} catch (const std::exception &) {
			std::cerr << "Ignoring invalid " << name << "=" << value << '\n';
			return fallback;
		}
	}

	/**
	 * @brief Parses an on/off command-line value.
	 *
	 * @param value Token to parse.
	 * @param result Receives the parsed boolean.
	 * @return False when the token is not a recognised boolean.
	 */
	bool parseSwitch(const std::string &value, bool &result)
	{
		if (value == "on" || value == "1" || value == "true"
			|| value == "yes") {
			result = true;
			return true;
		}
		if (value == "off" || value == "0" || value == "false"
			|| value == "no") {
			result = false;
			return true;
		}
		return false;
	}

	/**
	 * @brief Parses a `WxH` window size.
	 *
	 * @param value Token to parse.
	 * @param width Receives the width in pixels.
	 * @param height Receives the height in pixels.
	 * @return False when the token is malformed or zero-sized.
	 */
	bool parseWindowSize(const std::string &value, std::size_t &width,
						 std::size_t &height)
	{
		const auto separator = value.find('x');

		if (separator == std::string::npos) {
			return false;
		}

		try {
			const long long parsedWidth =
				std::stoll(value.substr(0, separator));
			const long long parsedHeight =
				std::stoll(value.substr(separator + 1));

			if (parsedWidth <= 0 || parsedHeight <= 0) {
				return false;
			}
			width  = static_cast<std::size_t>(parsedWidth);
			height = static_cast<std::size_t>(parsedHeight);
			return true;
		} catch (const std::exception &) {
			return false;
		}
	}

	/**
	 * @brief Prints the command-line usage.
	 *
	 * @param program Executable name from argv[0].
	 */
	void printUsage(const char *program)
	{
		std::cout
			<< "Usage: " << program << " [options]\n"
			<< "\n"
			<< "Runs one measured stress configuration and prints a machine\n"
			<< "readable 'evan stress:' record. Sweep with "
			   "scripts/run-stress.sh.\n"
			<< "\n"
			<< "Options (environment fallbacks in brackets):\n"
			<< "  --meshes N           objects in the scene "
			   "[EVAN_STRESS_MESHES, 1000]\n"
			<< "  --pipelines K        distinct pipelines used "
			   "[EVAN_STRESS_PIPELINES, 1]\n"
			<< "  --frames F           measured frames "
			   "[EVAN_STRESS_FRAMES, 300]\n"
			<< "  --warmup W           warm-up frames "
			   "[EVAN_STRESS_WARMUP, 60]\n"
			<< "  --instancing on|off  merge identical draws "
			   "[EVAN_STRESS_INSTANCING, off]\n"
			<< "  --indirect on|off    indirect draws "
			   "[EVAN_STRESS_INDIRECT, off]\n"
			<< "  --culling on|off     frustum culling "
			   "[EVAN_STRESS_CULLING, on]\n"
			<< "  --present-mode M     fifo|mailbox|immediate|auto "
			   "[EVAN_PRESENT_MODE, fifo]\n"
			<< "  --window WxH         window size [800x600]\n"
			<< "  --csv FILE           append one CSV row per run\n"
			<< "  --quiet              no progress lines on stderr\n"
			<< "  -h, --help           show this help\n";
	}

	/**
	 * @brief Parses the command line into @p configuration.
	 *
	 * Environment variables provide the defaults, so a sweep script can set
	 * them once and override individual runs with flags.
	 *
	 * @param argc Argument count.
	 * @param argv Argument values.
	 * @param configuration Receives the parsed configuration.
	 * @return False when the arguments are invalid or help was requested.
	 */
	bool parseArguments(int argc, char **argv, Configuration &configuration)
	{
		configuration.meshes =
			readCountEnv("EVAN_STRESS_MESHES", configuration.meshes);
		configuration.pipelines =
			readCountEnv("EVAN_STRESS_PIPELINES", configuration.pipelines);
		configuration.frames =
			readCountEnv("EVAN_STRESS_FRAMES", configuration.frames);
		configuration.warmup =
			readCountEnv("EVAN_STRESS_WARMUP", configuration.warmup);

		if (const char *value = std::getenv("EVAN_STRESS_INSTANCING");
			value != nullptr && value[0] != '\0') {
			parseSwitch(value, configuration.instancing);
		}
		if (const char *value = std::getenv("EVAN_STRESS_INDIRECT");
			value != nullptr && value[0] != '\0') {
			parseSwitch(value, configuration.indirect);
		}
		if (const char *value = std::getenv("EVAN_STRESS_CULLING");
			value != nullptr && value[0] != '\0') {
			parseSwitch(value, configuration.culling);
		}
		if (const char *value = std::getenv("EVAN_PRESENT_MODE");
			value != nullptr && value[0] != '\0') {
			configuration.presentMode = value;
		}

		for (int index = 1; index < argc; ++index) {
			const std::string argument = argv[index];

			if (argument == "--quiet") {
				configuration.verbose = false;
				continue;
			}

			if (index + 1 >= argc) {
				std::cerr << argument << " requires a value\n";
				return false;
			}
			const std::string value = argv[++index];

			try {
				if (argument == "--meshes") {
					configuration.meshes =
						static_cast<std::size_t>(std::stoull(value));
				} else if (argument == "--pipelines") {
					configuration.pipelines =
						static_cast<std::size_t>(std::stoull(value));
				} else if (argument == "--frames") {
					configuration.frames =
						static_cast<std::size_t>(std::stoull(value));
				} else if (argument == "--warmup") {
					configuration.warmup =
						static_cast<std::size_t>(std::stoull(value));
				} else if (argument == "--instancing") {
					if (!parseSwitch(value, configuration.instancing)) {
						std::cerr << "invalid --instancing value: " << value
								  << '\n';
						return false;
					}
				} else if (argument == "--indirect") {
					if (!parseSwitch(value, configuration.indirect)) {
						std::cerr << "invalid --indirect value: " << value
								  << '\n';
						return false;
					}
				} else if (argument == "--culling") {
					if (!parseSwitch(value, configuration.culling)) {
						std::cerr << "invalid --culling value: " << value
								  << '\n';
						return false;
					}
				} else if (argument == "--present-mode") {
					configuration.presentMode = value;
				} else if (argument == "--window") {
					if (!parseWindowSize(value, configuration.width,
										 configuration.height)) {
						std::cerr << "invalid --window value: " << value
								  << " (expected WxH)\n";
						return false;
					}
				} else if (argument == "--csv") {
					configuration.csvPath = value;
				} else {
					std::cerr << "unknown option: " << argument << '\n';
					return false;
				}
			} catch (const std::exception &) {
				std::cerr << "invalid value for " << argument << ": " << value
						  << '\n';
				return false;
			}
		}

		configuration.pipelines =
			std::clamp<std::size_t>(configuration.pipelines, 1, kMaxPipelines);
		configuration.frames = std::max<std::size_t>(configuration.frames, 1);

		return true;
	}

	/**
	 * @brief Builds a plain white cube centered on the origin.
	 *
	 * Geometry stays in local space: the object transform places it, which is
	 * the workload the engine's per-instance path is built for.
	 *
	 * @return A mesh for the cube.
	 */
	utility::graphic::Mesh buildUnitCubeMesh()
	{
		constexpr float half = 0.5f;
		const utility::graphic::Color32Bit white(255, 255, 255, 255);

		utility::graphic::Mesh mesh(std::vector<utility::graphic::VertexF> {},
									std::vector<uint32_t> {});

		const auto addVertex = [&](float x, float y, float z) {
			utility::graphic::VertexF vertex;
			vertex.setPosition(utility::graphic::PositionF(x, y, z));
			vertex.setColor(white);
			mesh.addVertex(vertex);
		};

		addVertex(-half, -half, -half);	   // 0
		addVertex(half, -half, -half);	   // 1
		addVertex(half, half, -half);	   // 2
		addVertex(-half, half, -half);	   // 3
		addVertex(-half, -half, half);	   // 4
		addVertex(half, -half, half);	   // 5
		addVertex(half, half, half);	   // 6
		addVertex(-half, half, half);	   // 7

		// 12 triangles (36 indices), CCW as seen from outside.
		for (const uint32_t index:
			 { 0u, 3u, 2u, 0u, 2u, 1u,		  // back   (-Z)
			   4u, 5u, 6u, 4u, 6u, 7u,		  // front  (+Z)
			   0u, 1u, 5u, 0u, 5u, 4u,		  // bottom (-Y)
			   3u, 7u, 6u, 3u, 6u, 2u,		  // top    (+Y)
			   0u, 4u, 7u, 0u, 7u, 3u,		  // left   (-X)
			   1u, 2u, 6u, 1u, 6u, 5u }) {	  // right  (+X)
			mesh.addIndex(index);
		}

		return mesh;
	}

	/**
	 * @brief Owns and removes the scratch directory of generated shaders.
	 */
	class ScratchDirectory
	{
		public:
		/**
		 * @brief Creates a unique scratch directory under the temp folder.
		 *
		 * @param tag Name prefix, so concurrent runs do not collide.
		 */
		explicit ScratchDirectory(const std::string &tag)
			: _path(std::filesystem::temp_directory_path()
					/ ("evan-stress-" + tag + "-"
					   + std::to_string(currentProcessId())))
		{
			std::error_code error;

			std::filesystem::remove_all(_path, error);
			std::filesystem::create_directories(_path, error);
			if (error) {
				std::cerr << "cannot create " << _path.string() << ": "
						  << error.message() << '\n';
			}
		}

		~ScratchDirectory()
		{
			std::error_code error;

			std::filesystem::remove_all(_path, error);
		}

		ScratchDirectory(const ScratchDirectory &)			  = delete;
		ScratchDirectory &operator=(const ScratchDirectory &) = delete;
		ScratchDirectory(ScratchDirectory &&)				  = delete;
		ScratchDirectory &operator=(ScratchDirectory &&)	  = delete;

		/**
		 * @brief Retrieves the directory path.
		 *
		 * @return The scratch directory.
		 */
		[[nodiscard]] const std::filesystem::path &path() const noexcept
		{
			return _path;
		}

		/**
		 * @brief Reports whether the directory is usable.
		 *
		 * @return True when it exists.
		 */
		[[nodiscard]] bool valid() const noexcept
		{
			std::error_code error;

			return std::filesystem::is_directory(_path, error);
		}

		private:
		std::filesystem::path _path;
	};

	/**
	 * @brief Registers the shader variants that back the pipeline count.
	 *
	 * The engine creates one graphics pipeline per shader registered on the
	 * provider when the Renderer is constructed. To vary the pipeline count
	 * without recompiling, the harness copies the mesh shader's SPIR-V under
	 * distinct names and registers each copy, so every variant is its own
	 * shader module and its own pipeline.
	 *
	 * @param provider Provider that will be handed to the Engine.
	 * @param count Number of pipelines (the first one is the stock mesh
	 * shader).
	 * @param scratch Scratch directory receiving the copies.
	 * @return The shader name to bind for each pipeline, or an empty vector
	 * when a variant could not be registered.
	 */
	std::vector<std::string>
		registerShaderVariants(utility::RessourceProvider &provider,
							   std::size_t count,
							   const ScratchDirectory &scratch)
	{
		std::vector<std::string> names;

		if (count == 0) {
			return names;
		}

		names.emplace_back("mesh");	   // stock shader loaded by the Engine

		if (count == 1) {
			return names;
		}

		const std::filesystem::path vertexSource =
			std::filesystem::path("shaders") / "mesh.vert.spv";
		const std::filesystem::path fragmentSource =
			std::filesystem::path("shaders") / "mesh.frag.spv";

		if (!std::filesystem::exists(vertexSource)
			|| !std::filesystem::exists(fragmentSource)) {
			std::cerr << "missing mesh shader in shaders/ (run from the "
						 "example directory)\n";
			return {};
		}

		for (std::size_t index = 1; index < count; ++index) {
			const std::string stem	= "stress" + std::to_string(index);
			const auto vertexPath	= scratch.path() / (stem + ".vert.spv");
			const auto fragmentPath = scratch.path() / (stem + ".frag.spv");

			std::error_code error;

			std::filesystem::copy_file(
				vertexSource, vertexPath,
				std::filesystem::copy_options::overwrite_existing, error);
			if (!error) {
				std::filesystem::copy_file(
					fragmentSource, fragmentPath,
					std::filesystem::copy_options::overwrite_existing, error);
			}
			if (error) {
				std::cerr << "cannot create shader variant " << stem << ": "
						  << error.message() << '\n';
				return {};
			}

			if (!provider.loadShader(vertexPath.string(),
									 fragmentPath.string())) {
				std::cerr << "cannot register shader variant " << stem << '\n';
				return {};
			}
			names.push_back(stem);
		}

		return names;
	}

	/**
	 * @brief Registers @p count cubes on a grid in front of the camera.
	 *
	 * Objects cycle through @p shaderNames so the frame's draw list spans the
	 * requested number of pipelines. The grid is fitted to the live view, so
	 * every cube stays inside the frustum and culling does not silently shrink
	 * the workload.
	 *
	 * @param engine Engine receiving the objects.
	 * @param count Number of cubes.
	 * @param shaderNames Shader to bind per cube, cycled.
	 * @return The number of objects actually created.
	 */
	std::size_t spawnCubeGrid(evan::Engine &engine, std::size_t count,
							  const std::vector<std::string> &shaderNames)
	{
		if (count == 0 || shaderNames.empty()) {
			return 0;
		}

		constexpr float spacing	  = 2.0f;
		constexpr float gridUsage = 0.85f;

		const auto view		= engine.getView();
		const auto viewport = view.getViewportSize();
		const float aspect = viewport.y > 0.0f ? viewport.x / viewport.y : 1.0f;
		float tanHalfVertical = std::tan(view.getVerticalFovRadians() * 0.5f);

		if (tanHalfVertical <= 0.0f) {
			tanHalfVertical = 1.0f;
		}

		const auto columns = static_cast<std::size_t>(
			std::ceil(std::sqrt(static_cast<double>(count))));
		const auto rows = (count + columns - 1) / columns;

		const float halfWidth  = static_cast<float>(columns) * spacing * 0.5f;
		const float halfHeight = static_cast<float>(rows) * spacing * 0.5f;
		const float distance   = std::max(
			  view.getNearPlane() * 2.0f,
			  std::max(halfWidth / (gridUsage * tanHalfVertical * aspect),
					   halfHeight / (gridUsage * tanHalfVertical)));

		const utility::graphic::Mesh cubeMesh = buildUnitCubeMesh();
		std::size_t created					  = 0;

		for (std::size_t index = 0; index < count; ++index) {
			const std::size_t objectID =
				engine.addMesh(cubeMesh, "default_material",
							   shaderNames[index % shaderNames.size()]);

			if (objectID == 0) {
				continue;
			}
			++created;

			const auto column = static_cast<float>(index % columns);
			const auto row	  = static_cast<float>(index / columns);
			const float x =
				(column - (static_cast<float>(columns) - 1.0f) * 0.5f)
				* spacing;
			const float y =
				(row - (static_cast<float>(rows) - 1.0f) * 0.5f) * spacing;

			engine.setObjectTransform(
				objectID,
				glm::translate(glm::mat4(1.0f), glm::vec3(x, y, -distance)));
		}

		return created;
	}

	/**
	 * @brief Returns the value at a percentile of an already sorted sample.
	 *
	 * Uses the nearest-rank definition, so the result is always one of the
	 * measured samples.
	 *
	 * @param sorted Frame times in milliseconds, ascending.
	 * @param fraction Fraction in (0, 1].
	 * @return The percentile value, or 0 for an empty sample.
	 */
	double percentile(const std::vector<double> &sorted, double fraction)
	{
		if (sorted.empty()) {
			return 0.0;
		}

		const double rank =
			std::ceil(fraction * static_cast<double>(sorted.size()));
		const auto index = static_cast<std::size_t>(std::max(1.0, rank)) - 1;

		return sorted[std::min(index, sorted.size() - 1)];
	}

	/**
	 * @brief Summarises the measured frame times.
	 *
	 * @param frameMs Frame times of the measured frames, in milliseconds.
	 * @return The frame rate and the frame-time distribution.
	 */
	Measurement summarize(std::vector<double> frameMs)
	{
		Measurement measurement;

		if (frameMs.empty()) {
			return measurement;
		}

		double total = 0.0;

		for (const double value: frameMs) {
			total += value;
		}

		std::sort(frameMs.begin(), frameMs.end());

		measurement.elapsedSec = total / 1000.0;
		measurement.fps		   = total > 0.0
				   ? static_cast<double>(frameMs.size()) * 1000.0 / total
				   : 0.0;
		measurement.p50ms	   = percentile(frameMs, 0.50);
		measurement.p95ms	   = percentile(frameMs, 0.95);
		measurement.p99ms	   = percentile(frameMs, 0.99);
		measurement.maxMs	   = frameMs.back();
		measurement.fpsP50 =
			measurement.p50ms > 0.0 ? 1000.0 / measurement.p50ms : 0.0;
		measurement.fpsLow1pct =
			measurement.maxMs > 0.0 ? 1000.0 / measurement.maxMs : 0.0;

		return measurement;
	}

	/**
	 * @brief Converts nanoseconds to milliseconds.
	 *
	 * @param ns Duration in nanoseconds.
	 * @return The duration in milliseconds.
	 */
	double nanosecondsToMilliseconds(uint64_t ns)
	{
		return static_cast<double>(ns) / 1.0e6;
	}

	/**
	 * @brief Formats a floating-point value with a fixed precision.
	 *
	 * @param value Value to format.
	 * @param precision Digits after the decimal point.
	 * @return The formatted value.
	 */
	std::string format(double value, int precision = 2)
	{
		char buffer[64];

		std::snprintf(buffer, sizeof(buffer), "%.*f", precision, value);
		return buffer;
	}

	/**
	 * @brief Per-frame value of a window counter.
	 *
	 * The renderer's counters are sums over the report window, so they are
	 * divided by the window's frame count before being reported.
	 *
	 * @param stats Renderer statistics window.
	 * @param value Window counter.
	 * @return The per-frame value.
	 */
	double perFrame(const evan::FrameStats &stats, uint64_t value)
	{
		return stats.frames > 0
			? static_cast<double>(value) / static_cast<double>(stats.frames)
			: 0.0;
	}

	/**
	 * @brief Prints the machine-readable record of one run.
	 *
	 * The keys are stable: scripts/run-stress.sh parses this line.
	 *
	 * @param configuration Configuration of the run.
	 * @param measurement Measured frame rate and frame times.
	 * @param created Objects actually created.
	 * @param initMs Engine construction time, in milliseconds.
	 * @param sceneMs Scene construction time, in milliseconds.
	 * @param stats Renderer statistics window, or nullptr when disabled.
	 */
	void printRecord(const Configuration &configuration,
					 const Measurement &measurement, std::size_t created,
					 double initMs, double sceneMs,
					 const evan::FrameStats *stats)
	{
		std::cout << "evan stress: meshes=" << configuration.meshes
				  << " created=" << created
				  << " pipelines=" << configuration.pipelines
				  << " frames=" << configuration.frames
				  << " warmup=" << configuration.warmup
				  << " fps=" << format(measurement.fps)
				  << " fps_p50=" << format(measurement.fpsP50)
				  << " fps_1pct_low=" << format(measurement.fpsLow1pct)
				  << " frame_ms_p50=" << format(measurement.p50ms)
				  << " frame_ms_p95=" << format(measurement.p95ms)
				  << " frame_ms_p99=" << format(measurement.p99ms)
				  << " frame_ms_max=" << format(measurement.maxMs)
				  << " elapsed_s=" << format(measurement.elapsedSec, 3)
				  << " init_ms=" << format(initMs, 1)
				  << " scene_ms=" << format(sceneMs, 1) << " present_mode="
				  << (configuration.presentMode.empty()
						  ? "default"
						  : configuration.presentMode)
				  << " instancing=" << (configuration.instancing ? 1 : 0)
				  << " indirect=" << (configuration.indirect ? 1 : 0)
				  << " culling=" << (configuration.culling ? 1 : 0)
				  << " stats=" << (stats != nullptr ? 1 : 0);

		if (stats == nullptr) {
			std::cout << '\n';
			return;
		}

		std::cout
			<< " stat_frames=" << stats->frames
			<< " vis=" << format(perFrame(*stats, stats->visibleMeshes), 0)
			<< " culled=" << format(perFrame(*stats, stats->culledMeshes), 0)
			<< " skip=" << format(perFrame(*stats, stats->skippedMeshes), 0)
			<< " draws=" << format(perFrame(*stats, stats->drawCalls), 0)
			<< " instanced="
			<< format(perFrame(*stats, stats->instancedDraws), 0)
			<< " pso_binds="
			<< format(perFrame(*stats, stats->pipelineBinds), 0) << " ds_binds="
			<< format(perFrame(*stats, stats->descriptorBinds), 0)
			<< " sync_calls=" << format(perFrame(*stats, stats->syncCalls), 0)
			<< " sync_skip=" << format(perFrame(*stats, stats->syncSkips), 0)
			<< " upload_waits="
			<< format(perFrame(*stats, stats->uploadWaits), 0) << " rec_ms="
			<< format(nanosecondsToMilliseconds(stats->record.averageNs()))
			<< " submit_ms="
			<< format(nanosecondsToMilliseconds(stats->submit.averageNs()))
			<< " wait_ms="
			<< format(
				   nanosecondsToMilliseconds(stats->inFlightWait.averageNs()))
			<< " ubo_ms="
			<< format(
				   nanosecondsToMilliseconds(stats->updateUniform.averageNs()))
			<< " pres_ms="
			<< format(nanosecondsToMilliseconds(stats->present.averageNs()))
			<< " gpu_ms="
			<< format(nanosecondsToMilliseconds(stats->gpuPass[0].averageNs()))
			<< '\n';
	}

	/**
	 * @brief Appends one CSV row to @p path, writing the header on first use.
	 *
	 * @param path Destination file.
	 * @param configuration Configuration of the run.
	 * @param measurement Measured frame rate and frame times.
	 * @param created Objects actually created.
	 * @param initMs Engine construction time, in milliseconds.
	 * @param sceneMs Scene construction time, in milliseconds.
	 * @param stats Renderer statistics window, or nullptr when disabled.
	 */
	void appendCsv(const std::string &path, const Configuration &configuration,
				   const Measurement &measurement, std::size_t created,
				   double initMs, double sceneMs, const evan::FrameStats *stats)
	{
		std::ifstream probe(path, std::ios::binary);
		const bool needsHeader = !probe.good() || probe.peek() == EOF;

		probe.close();

		std::ofstream csv(path, std::ios::app);

		if (!csv) {
			std::cerr << "cannot open " << path << " for writing\n";
			return;
		}

		if (needsHeader) {
			csv << "meshes,created,pipelines,frames,warmup,fps,fps_p50,fps_"
				   "1pct_low,"
				   "frame_ms_p50,frame_ms_p95,frame_ms_p99,frame_ms_max,"
				   "init_ms,scene_ms,present_mode,instancing,indirect,"
				   "culling,draws,instanced,pso_binds,ds_binds,vis,culled,"
				   "skip,rec_ms,submit_ms,wait_ms,ubo_ms,pres_ms,gpu_ms,"
				   "upload_waits\n";
		}

		const auto value = [stats](uint64_t counter) {
			return stats != nullptr ? perFrame(*stats, counter) : 0.0;
		};
		const auto phase = [](const evan::PhaseStats &phaseStats) {
			return nanosecondsToMilliseconds(phaseStats.averageNs());
		};

		csv << configuration.meshes << ',' << created << ','
			<< configuration.pipelines << ',' << configuration.frames << ','
			<< configuration.warmup << ',' << format(measurement.fps) << ','
			<< format(measurement.fpsP50) << ','
			<< format(measurement.fpsLow1pct) << ','
			<< format(measurement.p50ms) << ',' << format(measurement.p95ms)
			<< ',' << format(measurement.p99ms) << ','
			<< format(measurement.maxMs) << ',' << format(initMs, 1) << ','
			<< format(sceneMs, 1) << ','
			<< (configuration.presentMode.empty() ? "default"
												  : configuration.presentMode)
			<< ',' << (configuration.instancing ? 1 : 0) << ','
			<< (configuration.indirect ? 1 : 0) << ','
			<< (configuration.culling ? 1 : 0) << ','
			<< format(value(stats != nullptr ? stats->drawCalls : 0), 1) << ','
			<< format(value(stats != nullptr ? stats->instancedDraws : 0), 1)
			<< ','
			<< format(value(stats != nullptr ? stats->pipelineBinds : 0), 1)
			<< ','
			<< format(value(stats != nullptr ? stats->descriptorBinds : 0), 1)
			<< ','
			<< format(value(stats != nullptr ? stats->visibleMeshes : 0), 1)
			<< ','
			<< format(value(stats != nullptr ? stats->culledMeshes : 0), 1)
			<< ','
			<< format(value(stats != nullptr ? stats->skippedMeshes : 0), 1)
			<< ',' << format(stats != nullptr ? phase(stats->record) : 0.0)
			<< ',' << format(stats != nullptr ? phase(stats->submit) : 0.0)
			<< ','
			<< format(stats != nullptr ? phase(stats->inFlightWait) : 0.0)
			<< ','
			<< format(stats != nullptr ? phase(stats->updateUniform) : 0.0)
			<< ',' << format(stats != nullptr ? phase(stats->present) : 0.0)
			<< ',' << format(stats != nullptr ? phase(stats->gpuPass[0]) : 0.0)
			<< ','
			<< format(value(stats != nullptr ? stats->uploadWaits : 0), 1)
			<< '\n';
	}
}	 // namespace

int main(int argc, char **argv)
{
	Configuration configuration;

	for (int index = 1; index < argc; ++index) {
		if (std::strcmp(argv[index], "-h") == 0
			|| std::strcmp(argv[index], "--help") == 0) {
			printUsage(argv[0]);
			return EXIT_SUCCESS;
		}
	}

	if (!parseArguments(argc, argv, configuration)) {
		return EXIT_FAILURE;
	}

	// Published to the engine before anything is created: the present mode is
	// read when the swapchain is built, culling and the statistics interval
	// when the Renderer is built. An explicit user value always wins.
	if (!configuration.presentMode.empty()) {
		setEnvIfUnset("EVAN_PRESENT_MODE", configuration.presentMode);
	}
	if (!configuration.culling) {
		setEnvIfUnset("EVAN_DEBUG_DISABLE_CULLING", "1");
	}
	setEnvIfUnset("EVAN_STATS", "1");
	setEnvIfUnset(
		"EVAN_STATS_INTERVAL",
		std::to_string(configuration.warmup + configuration.frames + 1));

	const ScratchDirectory scratch("scene");

	if (!scratch.valid()) {
		std::cerr << "cannot create a scratch directory for shader variants\n";
		return EXIT_FAILURE;
	}

#if defined(__APPLE__)
	auto platform = std::make_shared<evan::MacOsDesktopPlatform>(
		"evan stress", static_cast<int>(configuration.width),
		static_cast<int>(configuration.height));
#elif defined(__linux__)
	auto platform = std::make_shared<evan::LinuxDesktopPlatform>(
		"evan stress", static_cast<int>(configuration.width),
		static_cast<int>(configuration.height));
#else
	auto platform = std::make_shared<evan::WindowsPlatform>(
		"evan stress", static_cast<int>(configuration.width),
		static_cast<int>(configuration.height));
#endif

	auto systemIO = std::make_shared<utility::DefaultSystemIO>();
	auto ressourceProvider =
		std::make_shared<utility::RessourceProvider>(*systemIO);

	if (configuration.verbose) {
		std::cerr << "stress: meshes=" << configuration.meshes
				  << " pipelines=" << configuration.pipelines
				  << " frames=" << configuration.frames
				  << " warmup=" << configuration.warmup << '\n';
	}

	const std::vector<std::string> shaderNames = registerShaderVariants(
		*ressourceProvider, configuration.pipelines, scratch);

	if (shaderNames.size() != configuration.pipelines) {
		std::cerr << "cannot register " << configuration.pipelines
				  << " shader variant(s)\n";
		return EXIT_FAILURE;
	}

	const auto initStart = std::chrono::steady_clock::now();
	evan::Engine engine(ressourceProvider, platform);
	const auto initEnd = std::chrono::steady_clock::now();

	engine.setInstancingEnabled(configuration.instancing);
	engine.setIndirectDrawingEnabled(configuration.indirect);

	const auto sceneStart = std::chrono::steady_clock::now();
	const std::size_t created =
		spawnCubeGrid(engine, configuration.meshes, shaderNames);
	const auto sceneEnd = std::chrono::steady_clock::now();

	const double initMs =
		std::chrono::duration<double, std::milli>(initEnd - initStart).count();
	const double sceneMs =
		std::chrono::duration<double, std::milli>(sceneEnd - sceneStart)
			.count();

	if (configuration.verbose) {
		std::cerr << "stress: created " << created << " object(s); init "
				  << format(initMs, 1) << " ms, scene " << format(sceneMs, 1)
				  << " ms\n";
	}

	const std::size_t totalFrames = configuration.warmup + configuration.frames;
	std::vector<double> frameMs;

	frameMs.reserve(configuration.frames);

	for (std::size_t frame = 0; frame < totalFrames; ++frame) {
		const auto frameStart = std::chrono::steady_clock::now();

		engine.update();
		engine.render();
		engine.pollEvents();

		const auto frameEnd = std::chrono::steady_clock::now();
		const double elapsedMs =
			std::chrono::duration<double, std::milli>(frameEnd - frameStart)
				.count();

		// Reset once, right after the warm-up: the reported window then covers
		// exactly the measured frames.
		if (frame + 1 == configuration.warmup) {
			engine.resetFrameStats();
		}
		if (frame >= configuration.warmup) {
			frameMs.push_back(elapsedMs);
		}

		if (configuration.verbose && (frame + 1) % 120 == 0) {
			std::cerr << "stress: " << (frame + 1) << "/" << totalFrames
					  << " frames\n";
		}
	}

	if (created == 0 && configuration.meshes != 0) {
		std::cerr << "warning: no object could be created\n";
	}

	const Measurement measurement  = summarize(frameMs);
	const evan::FrameStats &window = engine.getFrameStats();
	const evan::FrameStats *stats  = window.enabled ? &window : nullptr;

	printRecord(configuration, measurement, created, initMs, sceneMs, stats);

	if (!configuration.csvPath.empty()) {
		appendCsv(configuration.csvPath, configuration, measurement, created,
				  initMs, sceneMs, stats);
	}

	// The teardown path can abort on some drivers, so flush the record before
	// the engine destructors run.
	std::cout.flush();
	return EXIT_SUCCESS;
}
