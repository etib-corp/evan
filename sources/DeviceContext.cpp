/*
** ETIB PROJECT, 2026
** evan
** File description:
** DeviceContext
*/

#include "DeviceContext.hpp"

evan::DeviceContext::DeviceContext()
{
}

evan::DeviceContext::~DeviceContext()
{
    vkDestroyCommandPool(_device, _commandPool, nullptr);
    vkDestroyDevice(_device, nullptr);
    vkDestroyInstance(_VkInstance, nullptr);
}
