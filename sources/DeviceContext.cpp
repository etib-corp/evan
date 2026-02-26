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

VkSampleCountFlagBits evan::DeviceContext::getMsaaSamples() const
{
    return _msaaSamples;
}

void evan::DeviceContext::getMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(_physicalDevice, &physicalDeviceProperties);
	VkSampleCountFlags counts =
		physicalDeviceProperties.limits.framebufferColorSampleCounts
		& physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    switch (counts) {
        case VK_SAMPLE_COUNT_64_BIT:
            _msaaSamples = VK_SAMPLE_COUNT_64_BIT;
            break;
        case VK_SAMPLE_COUNT_32_BIT:
            _msaaSamples = VK_SAMPLE_COUNT_32_BIT;
            break;
        case VK_SAMPLE_COUNT_16_BIT:
            _msaaSamples = VK_SAMPLE_COUNT_16_BIT;
            break;
        case VK_SAMPLE_COUNT_8_BIT:
            _msaaSamples = VK_SAMPLE_COUNT_8_BIT;
            break;
        case VK_SAMPLE_COUNT_4_BIT:
            _msaaSamples = VK_SAMPLE_COUNT_4_BIT;
            break;
        case VK_SAMPLE_COUNT_2_BIT:
            _msaaSamples = VK_SAMPLE_COUNT_2_BIT;
            break;
        default:
            _msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    }
}

void evan::DeviceContext::createCommandPool()
{
    evan::QueueFamilyIndices queueFamilyIndices = _deviceBackend->findQueueFamilies(_physicalDevice);


    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); 
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult result = vkCreateCommandPool(_device, &commandPoolCreateInfo, nullptr, &_commandPool);
    if (result != VK_SUCCESS) {
        // TODO: replace with proper error handling
        std::cerr << "Failed to create command pool: " << result << std::endl;
        return;
    }
}