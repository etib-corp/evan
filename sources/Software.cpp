/*
** ETIB PROJECT, 2026
** evan
** File description:
** Software
*/

#include "Software.hpp"

std::unique_ptr<utility::AssetManager> g_assetManager =
	std::make_unique<utility::DefaultAssetManager>();

evan::Software::Software(const std::shared_ptr<IPlatform> &platform)
	: _platform(platform)
{
	g_assetManager = std::make_unique<utility::DefaultAssetManager>();
	g_assetManager->loadDirectory(std::string("./shaders"));

	_deviceContext	   = std::make_shared<DeviceContext>(*platform);
	_swapchainContext = platform->createSwapchainContext(*_deviceContext);

	auto deviceBackend = _deviceContext->getDeviceBackend();
	_renderer		   = std::make_shared<Renderer>(*_deviceContext,
													_swapchainContext->getRenderPass(),
													_deviceContext->getMsaaSamples());
	_currentScene	   = 0;

	for (int frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++) {
		_renderer->createFrame(_deviceContext->getCommandPool(),
							   *deviceBackend);
	}
}

evan::Software::~Software()
{
	auto deviceBackend = _deviceContext->getDeviceBackend();
	auto device		   = deviceBackend->_device;

	vkDeviceWaitIdle(device);

	_renderer->destroy(device);
	_swapchainContext->destroy(device);
	for (auto &scene: _scenes) {
		scene.destroy(device);
	}
	_deviceContext.reset();
}

////////////////////
// Public Methods //
////////////////////

void evan::Software::addScene(std::vector<std::string> texturePaths,
							  std::map<std::string, std::vector<Mesh>> meshData)
{
	_scenes.emplace_back(*_deviceContext, *_renderer, texturePaths, meshData);
	if (_scenes.size() == 1) {
		_currentScene = 0;
	}
}

void evan::Software::run()
{
	while (!_platform->shouldClose()) {
		_platform->pollEvents(*_deviceContext->getDeviceBackend());

		this->update();
		this->render();
	}
}

void evan::Software::update()
{
	// Logic updates, input handling, etc.
	// Will be implemented in the future when the input system and scene
	// management will be implemented.
}

void evan::Software::render()
{
	if (_scenes.empty()) {
		return;
	}

	_renderer->drawFrame(*_deviceContext, *_swapchainContext,
						 _scenes[_currentScene]);
}