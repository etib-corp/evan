/*
** ETIB PROJECT, 2026
** evan
** File description:
** Software
*/

#include "Software.hpp"

evan::Software::Software()
{
    #ifdef __OPENXR__
        _platform = std::make_shared<XRPlatform>(); // TODO: Pass the platform data
        _deviceContext = std::make_shared<DeviceContext>(*_platform);
        _swapchainContext = std::make_shared<XRSwapchainContext>(*_deviceContext);
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
    #ifdef __OPENXR__
        _platform = std::make_shared<XRPlatform>(); // TODO: Pass the platform data
        _deviceContext = std::make_shared<DeviceContext>(*_platform);
        _swapchainContext = std::make_shared<XRSwapchainContext>(*_deviceContext);
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
}

void evan::Software::addScene(std::vector<std::string> texturePaths, std::map<std::string, std::vector<Mesh>> meshData)
{
    _scenes.emplace_back(*_deviceContext, *_renderer, texturePaths, meshData);
}
