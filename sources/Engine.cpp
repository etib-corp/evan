/*
** ETIB PROJECT, 2026
** evan
** File description:
** Engine
*/

#include "Engine.hpp"

#ifdef __ANDROID__
std::unique_ptr<utility::AndroidAssetManager> g_assetManager =
	std::make_unique<utility::AndroidAssetManager>();
#else
std::unique_ptr<utility::AssetManager> g_assetManager =
	std::make_unique<utility::DefaultAssetManager>();
#endif

evan::Engine::Engine(const std::shared_ptr<IPlatform> &platform)
	: _platform(platform)
{
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

evan::Engine::~Engine()
{
	auto deviceBackend = _deviceContext->getDeviceBackend();
	auto device		   = deviceBackend->_device;

	vkDeviceWaitIdle(device);

	_renderer->destroy(device);
	_swapchainContext->destroy(device);
	for (auto &[_, scene]: _scenes) {
		scene.destroy(device);
	}
	_deviceContext.reset();
}

////////////////////
// Public Methods //
////////////////////

void evan::Engine::addScene(size_t sceneIndex, std::vector<std::string> texturePaths,
							std::map<std::string, std::vector<Mesh>> meshData)
{
	Scene newScene(*_deviceContext, *_renderer, texturePaths, meshData);
	_scenes.emplace(sceneIndex, std::move(newScene));
	if (_scenes.size() == 1) {
		_currentScene = sceneIndex;
	}
}

void evan::Engine::update()
{
	// Logic updates, input handling, etc.
	// Will be implemented in the future when the input system and scene
	// management will be implemented.
}

void evan::Engine::render()
{
	if (_scenes.empty()) {
		return;
	}

	auto currentSceneIt = _scenes.find(_currentScene);
	if (currentSceneIt == _scenes.end()) {
		return;
	}

	_renderer->drawFrame(*_deviceContext, *_swapchainContext,
						 currentSceneIt->second);
}

void evan::Engine::pollEvents()
{
	_platform->pollEvents(*_deviceContext->getDeviceBackend());
}

void evan::Engine::switchScene(size_t sceneIndex)
{
	if (_scenes.find(sceneIndex) != _scenes.end()) {
		_currentScene = sceneIndex;
	} else {
		std::cerr << "Scene index " << sceneIndex << " does not exist." << std::endl;
	}
}

void evan::Engine::updateScene(size_t sceneIndex, std::vector<std::string> texturePaths,
								std::map<std::string, std::vector<Mesh>> meshData)
{
	auto sceneIt = _scenes.find(sceneIndex);
	if (sceneIt != _scenes.end()) {
		sceneIt->second.updateScene(*_deviceContext, *_renderer, texturePaths, meshData);
	} else {
		std::cerr << "Scene index " << sceneIndex << " does not exist." << std::endl;
	}
}