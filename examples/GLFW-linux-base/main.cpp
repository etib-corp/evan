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

#include <cstdint>
#include <iostream>

#include <evan/glfw/platform/LinuxDesktopPlatform.hpp>

#include <evan/Engine.hpp>

#include <utility/graphic/color.hpp>
#include <utility/graphic/mesh.hpp>
#include <utility/graphic/position.hpp>
#include <utility/graphic/primitive.hpp>
#include <utility/graphic/vertex.hpp>

namespace
{
	/**
	 * @brief Builds a plain white cube placed in front of the viewer.
	 * @return A mesh for the cube, with vertices already in world space.
	 */
	utility::graphic::Mesh buildWhiteCubeMesh()
	{
		constexpr float half = 0.5f;
		const utility::graphic::PositionF center(5.0f, 5.0f, 0.0f);
		const utility::graphic::Color32Bit white(255, 255, 255, 255);

		utility::graphic::Mesh mesh(std::vector<utility::graphic::VertexF> {},
									std::vector<uint32_t> {});

		const auto addVertex = [&](float x, float y, float z) {
			utility::graphic::VertexF vertex;
			vertex.setPosition(utility::graphic::PositionF(
				center.x + x, center.y + y, center.z + z));
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
}	 // namespace

int main(void)
{
	auto platform =
		std::make_shared<evan::LinuxDesktopPlatform>("linux base", 800, 600);

	auto systemIO = std::make_shared<utility::DefaultSystemIO>();
	auto ressourceProvider =
		std::make_shared<utility::RessourceProvider>(*systemIO);

	evan::Engine engine(ressourceProvider, platform);

	auto cubePrimitive = std::make_shared<utility::graphic::Primitive>(
		std::vector<utility::graphic::Mesh> { buildWhiteCubeMesh() });
	engine.addPrimitive(cubePrimitive);

	while (!platform->shouldClose()) {
		engine.update();
		engine.render();
		engine.pollEvents();
	}
	return 0;
}
