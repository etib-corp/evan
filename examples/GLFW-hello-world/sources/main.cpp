#include <iostream>
#include <Software.hpp>
#include <Renderer.hpp>
#include <Frame.hpp>
#include <map>

int main(void)
{
	evan::Software mySoftware;

	std::vector<std::string> texturePaths = {
		"./texture1.png",
	};

	std::map<std::string, std::vector<evan::Mesh>> meshData = {
		{"./texture1.png", {evan::Mesh{std::vector<evan::Vertex>{
		evan::Vertex{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		evan::Vertex{{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		evan::Vertex{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		evan::Vertex{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},},
		std::vector<unsigned int>{0, 1, 2, 2, 3, 0}}}},
	};

	mySoftware.addScene(texturePaths, meshData);
	while (true) {
		mySoftware.render();
	}
	return 0;
}
