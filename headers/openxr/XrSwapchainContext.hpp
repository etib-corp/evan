/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrSwapchainContext
*/

#pragma once

#include "ASwapchainContext.hpp"
#include "ASwapchainImage.hpp"
#include "DeviceContext.hpp"
#include "openxr/XrDeviceBackend.hpp"
#include "openxr/XrSwapchainImage.hpp"

#include <map>
#include <memory>

namespace evan
{
    class XrSwapchainContext : public ASwapchainContext
    {
    public:
        XrSwapchainContext(const DeviceContext& deviceContext);

        void destroy(VkDevice device) override;

        VkResult aquireImage(uint32_t index, VkDevice device, VkSemaphore imageAvailableSemaphore, VkFence inFlightFence, uint32_t &imageIndex) override;

    private:
        std::vector<XrViewConfigurationView> _viewsConfigurations;
        std::vector<XrView> _views; // Vector of XrView structures for each view configuration
    };
} // namespace evan