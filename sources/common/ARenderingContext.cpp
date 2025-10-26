/*
** ETIB PROJECT, 2025
** evan
** File description:
** ARenderingContext
*/

#include "ARenderingContext.hpp"

///////////////////////
// Protected methods //
///////////////////////

VkSampleCountFlagBits evan::ARenderingContext::getMaxUsableSampleCount() const {
  VkPhysicalDeviceProperties physicalDeviceProperties;
  vkGetPhysicalDeviceProperties(_physicalDevice, &physicalDeviceProperties);
  VkSampleCountFlags counts =
      physicalDeviceProperties.limits.framebufferColorSampleCounts &
      physicalDeviceProperties.limits.framebufferDepthSampleCounts;

  if (counts & VK_SAMPLE_COUNT_64_BIT)
    return VK_SAMPLE_COUNT_64_BIT;
  if (counts & VK_SAMPLE_COUNT_32_BIT)
    return VK_SAMPLE_COUNT_32_BIT;
  if (counts & VK_SAMPLE_COUNT_16_BIT)
    return VK_SAMPLE_COUNT_16_BIT;
  if (counts & VK_SAMPLE_COUNT_8_BIT)
    return VK_SAMPLE_COUNT_8_BIT;
  if (counts & VK_SAMPLE_COUNT_4_BIT)
    return VK_SAMPLE_COUNT_4_BIT;
  if (counts & VK_SAMPLE_COUNT_2_BIT)
    return VK_SAMPLE_COUNT_2_BIT;

  return VK_SAMPLE_COUNT_1_BIT;
}

void evan::ARenderingContext::createCommandPool() {
  if (_logicalDevice == VK_NULL_HANDLE) {
    std::cerr << "Logical device is not initialized" << std::endl;
    return;
  }

  if (_commandPool != VK_NULL_HANDLE) {
    return;
  }

  Utils::QueueFamilyIndices queueCreateInfo =
      Utils::findQueueFamilies(_physicalDevice);

  VkCommandPoolCreateInfo commandPoolCreateInfo{};
  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.queueFamilyIndex =
      queueCreateInfo.graphicsFamily.value();
  commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(_logicalDevice, &commandPoolCreateInfo, nullptr,
                          &_commandPool) != VK_SUCCESS) {
    std::cerr << "Failed to create Vulkan command pool" << std::endl;
    return;
  }
}
