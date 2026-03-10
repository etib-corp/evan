/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrSwapchainContext
*/

#pragma once

#include "ASwapchainContext.hpp"
#include "DeviceContext.hpp"

namespace evan
{
    class XrSwapchainContext : public ASwapchainContext
    {
    public:
        XrSwapchainContext(const DeviceContext& deviceContext);
    };
} // namespace evan