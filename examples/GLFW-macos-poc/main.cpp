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

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include <evan/glfw/platform/MacOsDesktopPlatform.hpp>

#include <evan/Engine.hpp>

#include <utility/graphic/color.hpp>
#include <utility/graphic/mesh.hpp>
#include <utility/graphic/primitive.hpp>
#include <utility/graphic/vertex.hpp>

namespace
{
	/**
	 * @brief Reads an unsigned count from an environment variable.
	 *
	 * Unset, empty or invalid values fall back to @p fallback, so the PoC
	 * knobs stay opt-in and the run can still be configured explicitly.
	 *
	 * @param name Environment variable name.
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
	 * @brief Builds a plain white cube centered on the origin.
	 *
	 * The geometry stays in local space: the transform uploaded by
	 * Engine::setObjectTransform places it, which is what the
	 * EVAN_POC_MESHES workload exercises.
	 *
	 * @return A mesh for the cube, in local space.
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

		// 8 corners of an axis-aligned cube.
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
	 * @brief Registers @p count cubes on a grid in front of the camera.
	 *
	 * Every object owns its registry entry, its own GPUMesh (two device
	 * allocations: vertex and index buffer) and its own draw call, which is
	 * the workload the P3 proof of concept measures. The grid is sized from
	 * the live view so that every cube stays inside the frustum at both
	 * N=100 and N=1000.
	 *
	 * @param engine Engine that receives the objects.
	 * @param count Number of cubes to spawn (0 leaves the scene empty).
	 */
	void spawnCubeGrid(evan::Engine &engine, std::size_t count)
	{
		if (count == 0) {
			return;
		}

		// One cube edge plus one cube of gap between neighbours.
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

		for (std::size_t index = 0; index < count; ++index) {
			const auto cube = std::make_shared<utility::graphic::Primitive>(
				std::vector<utility::graphic::Mesh> { cubeMesh });
			const std::size_t objectID = engine.createObject(cube);

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

		std::cout << "PoC workload: " << count << " cubes on a " << columns
				  << "x" << rows << " grid at z=-" << distance << '\n';
	}
}	 // namespace

int main(void)
{
	// Both knobs default to 0, which keeps the empty scene and the "run until
	// the window is closed" loop. EVAN_POC_FRAMES gives a run a fixed frame
	// budget, which makes the stats report windows reproducible.
	const std::size_t meshCount = readCountEnv("EVAN_POC_MESHES", 0);
	const std::size_t maxFrames = readCountEnv("EVAN_POC_FRAMES", 0);

	auto platform =
		std::make_shared<evan::MacOsDesktopPlatform>("macos poc", 800, 600);

	auto systemIO = std::make_shared<utility::DefaultSystemIO>();
	auto ressourceProvider =
		std::make_shared<utility::RessourceProvider>(*systemIO);

	evan::Engine engine(ressourceProvider, platform);

	spawnCubeGrid(engine, meshCount);

	std::size_t frame = 0;

	while (!platform->shouldClose()) {
		engine.update();
		engine.render();
		engine.pollEvents();

		++frame;
		if (maxFrames != 0 && frame >= maxFrames) {
			break;
		}
	}

	// The teardown path can abort (pre-existing engine-side cleanup), so flush
	// the buffered report before the destructors run.
	std::cout.flush();
	return 0;
}
