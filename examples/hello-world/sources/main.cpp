#include <iostream>
#include <Engine.hpp>
#include <Renderer.hpp>
#include <Frame.hpp>
#include <map>
#if defined(__OPENXR__)
	#include <openxr/platform/LinuxXrPlatform.hpp>
#elif defined(__APPLE__)
	#include <glfw/platform/MacOsDesktopPlatform.hpp>
#endif


int main(void)
{
	std::shared_ptr<evan::IPlatform> platform =
		std::make_shared<evan::MacOsDesktopPlatform>("test", 800, 600);
	evan::Engine myEngine(platform);

	g_assetManager->add(std::string("./texture1.png"));

	std::vector<std::string> texturePaths = {
		"./texture1.png",
	};

	std::map<std::string, std::vector<evan::Mesh>> meshData = {
		{ "./texture1.png",
		  { evan::Mesh { std::vector<evan::Vertex> {
							 evan::Vertex { { -0.5f, -0.5f, -2.0f },
											{ 0.0f, 0.0f, 0.0f },
											{ 0.0f, 0.0f } },
							 evan::Vertex { { 0.5f, -0.5f, -2.0f },
											{ 1.0f, 1.0f, 0.0f },
											{ 1.0f, 0.0f } },
							 evan::Vertex { { 0.5f, 0.5f, -2.0f },
											{ 1.0f, 1.0f, 0.0f },
											{ 1.0f, 1.0f } },
							 evan::Vertex { { -0.5f, 0.5f, -2.0f },
											{ 0.0f, 1.0f, 0.0f },
											{ 0.0f, 1.0f } },
						 },
						 std::vector<unsigned int> { 0, 1, 2, 2, 3, 0 } } } },
	};

	myEngine.addScene(0, texturePaths, meshData);
	while (!platform->shouldClose()) {
		myEngine.pollEvents();

		myEngine.update();
		myEngine.render();
	}
	return 0;
}
