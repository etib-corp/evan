/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrSwapchainContext
*/

#include "openxr/XrSwapchainContext.hpp"

evan::XrSwapchainContext::XrSwapchainContext(
    const DeviceContext &deviceContext)
{
    createRenderPass(deviceContext.getDeviceBackend(), deviceContext.getMsaaSamples());
}