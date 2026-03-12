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
        _swapchainContext = std::make_shared<XRSwapchainContext>(*_deviceContext)
    #elif defined(__GLFW__)
        _platform = std::make_shared<DesktopPlatform>("Evan app", 800, 600);
        _deviceContext = std::make_shared<DeviceContext>(*_platform);
        _swapchainContext = std::make_shared<DesktopSwapchainContext>(*_deviceContext)
    #else
        throw std::runtime_error("Unsupported platform");
    #endif
    auto device = _deviceContext->getDeviceBackend()->_device;

    _renderer = std::make_shared<Renderer>();
    _renderer->createGraphicsPipeline(device, _swapchainContext->getRenderPass(), _deviceContext->getMsaaSamples());
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        _renderer->createFrame(_deviceContext->getCommandPool(), *_deviceContext->getDeviceBackend());
    }
    _renderer->createDescriptorPool(device, 1000); // TODO: Change this with the AssetManager when it will be implemented
}

evan::Software::Software(const std::string &windowName, const uint32_t width, const uint32_t height)
{
    #ifdef __OPENXR__
        _platform = std::make_shared<XRPlatform>(); // TODO: Pass the platform data
        _deviceContext = std::make_shared<DeviceContext>(*_platform);
        _swapchainContext = std::make_shared<XRSwapchainContext>(*_deviceContext)
    #elif defined(__GLFW__)
        _platform = std::make_shared<DesktopPlatform>(windowName, width, height);
        _deviceContext = std::make_shared<DeviceContext>(*_platform);
        _swapchainContext = std::make_shared<DesktopSwapchainContext>(*_deviceContext)
    #else
        throw std::runtime_error("Unsupported platform");
    #endif
    auto device = _deviceContext->getDeviceBackend()->_device;

    _renderer = std::make_shared<Renderer>();
    _renderer->createGraphicsPipeline(device, _swapchainContext->getRenderPass(), _deviceContext->getMsaaSamples());
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        _renderer->createFrame(_deviceContext->getCommandPool(), *_deviceContext->getDeviceBackend());
    }
    _renderer->createDescriptorPool(device, 1000); // TODO: Change this with the AssetManager when it will be implemented
}

evan::Software::~Software()
{
}

void evan::Software::addScene(std::vector<std::string> texturePaths, std::map<std::string, std::vector<Mesh>> meshData)
{
    _scenes.emplace_back(*_deviceContext, *_renderer, texturePaths, meshData);
}
