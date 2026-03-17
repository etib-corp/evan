/*
** ETIB PROJECT, 2026
** evan
** File description:
** Software
*/

#include "Software.hpp"

std::unique_ptr<utility::AssetManager> g_assetManager = std::make_unique<utility::DefaultAssetManager>();

evan::Software::Software()
{
    g_assetManager->loadDirectory(std::string("./shaders"));
    #ifdef __OPENXR__
        _platform = std::make_shared<XrPlatform>(); // TODO: Pass the platform data
        _deviceContext = std::make_shared<DeviceContext>(*_platform);
        _swapchainContext = std::make_shared<XrSwapchainContext>(*_deviceContext);
    #elif defined(__GLFW__)
        _platform = std::make_shared<DesktopPlatform>("Evan app", 800, 600);
        _deviceContext = std::make_shared<DeviceContext>(*_platform);
        _swapchainContext = std::make_shared<DesktopSwapchainContext>(*_deviceContext, ((DesktopPlatform*)_platform.get())->_window);
    #else
        throw std::runtime_error("Unsupported platform");
    #endif
    auto deviceBackend = _deviceContext->getDeviceBackend();
    _renderer = std::make_shared<Renderer>(deviceBackend->_device, _swapchainContext->getRenderPass(), _deviceContext->getMsaaSamples());
}

evan::Software::Software(const std::string &windowName, const uint32_t width, const uint32_t height)
{
    g_assetManager = std::make_unique<utility::DefaultAssetManager>();
    g_assetManager->loadDirectory(std::string("./shaders"));
    #ifdef __OPENXR__
        _platform = std::make_shared<XrPlatform>(); // TODO: Pass the platform data
        _deviceContext = std::make_shared<DeviceContext>(*_platform);
        _swapchainContext = std::make_shared<XrSwapchainContext>(*_deviceContext);
    #elif defined(__GLFW__)
        _platform = std::make_shared<DesktopPlatform>(windowName, width, height);
        _deviceContext = std::make_shared<DeviceContext>(*_platform);
        _swapchainContext = std::make_shared<DesktopSwapchainContext>(*_deviceContext, ((DesktopPlatform*)_platform.get())->_window);
    #else
        throw std::runtime_error("Unsupported platform");
    #endif
    auto deviceBackend = _deviceContext->getDeviceBackend();
    _renderer = std::make_shared<Renderer>(deviceBackend->_device, _swapchainContext->getRenderPass(), _deviceContext->getMsaaSamples());
}

evan::Software::~Software()
{
    auto deviceBackend = _deviceContext->getDeviceBackend();
    auto device = deviceBackend->_device;

    vkDeviceWaitIdle(device);

    _renderer->destroy(device);
    _swapchainContext->destroy(device);
    for (auto &scene : _scenes) {
        scene.destroy(device);
    }
    _deviceContext.reset();
}

void evan::Software::addScene(std::vector<std::string> texturePaths, std::map<std::string, std::vector<Mesh>> meshData)
{
    _scenes.emplace_back(*_deviceContext, *_renderer, texturePaths, meshData);
}

void evan::Software::run()
{
    while (!_platform->shouldClose()) {
        _platform->pollEvents();

        this->update();
        this->render();
    }
}

void evan::Software::update()
{
    // Logic updates, input handling, etc.
    // Will be implemented in the future when the input system and scene management will be implemented.
}

void evan::Software::render()
{
    // Render the current scene.
}