/*
** ETIB PROJECT, 2025
** evan
** File description:
** ARenderingContext
*/

#pragma once

#include "Evan.hpp"
#include <array>
#include <memory>

#include "Error.hpp"
#include "Utils.hpp"

/**
 * @struct VulkanContext
 * @brief Represents the Vulkan rendering context, encapsulating essential
 * Vulkan objects and configurations.
 *
 * This structure is used to manage and store Vulkan-specific resources required
 * for rendering operations.
 *
 * @var VulkanContext::logicalDevice
 * The Vulkan logical device used for rendering operations.
 *
 * @var VulkanContext::physicalDevice
 * The Vulkan physical device representing the GPU hardware.
 *
 * @var VulkanContext::graphicsQueue
 * The Vulkan queue used for submitting graphics commands.
 *
 * @var VulkanContext::renderPass
 * The Vulkan render pass object defining the rendering pipeline.
 *
 * @var VulkanContext::commandPool
 * The Vulkan command pool used for allocating command buffers.
 *
 * @var VulkanContext::graphicsQueueFamilyIndex
 * The index of the queue family that supports graphics operations.
 */
#ifdef __GLFW__
struct VulkanContext {
  VkDevice logicalDevice;
  VkPhysicalDevice physicalDevice;
  VkQueue graphicsQueue;
  VkCommandPool commandPool;
  uint32_t graphicsQueueFamilyIndex;
  VkSurfaceKHR surface;
  GLFWwindow *window;
  VkSampleCountFlagBits msaaSamples;
};
#elif __OPENXR__
struct VulkanContext {
  VkDevice logicalDevice;
  VkPhysicalDevice physicalDevice;
  VkQueue graphicsQueue;
  VkCommandPool commandPool;
  uint32_t graphicsQueueFamilyIndex;
  VkSampleCountFlagBits msaaSamples;
};

#endif

namespace evan {

class ARenderingContextError : public evan::Error {
public:
  ARenderingContextError(const std::string &message) : evan::Error(message) {}
};

class ARenderingContext {
public:
  virtual ~ARenderingContext() = default;

  /**
   * @brief Retrieves the Vulkan context associated with this rendering context.
   *
   * @return A constant reference to a shared pointer of VulkanContext.
   */
  const std::shared_ptr<VulkanContext> &getVulkanContext() const {
    return _vulkanContext;
  }

private:
  /**
   * @brief Chooses the most suitable surface format for the swapchain.
   *
   * This function iterates through the list of available surface formats and
   * selects the one that best matches the desired format and color space.
   * If the preferred format (VK_FORMAT_B8G8R8A8_SRGB) and color space
   * (VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) are available, it returns that format.
   * Otherwise, it defaults to the first available format in the list.
   *
   * @param availableFormats A vector of VkSurfaceFormatKHR structures
   * representing the formats supported by the surface.
   * @return VkSurfaceFormatKHR The chosen surface format.
   */
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat : availableFormats) {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
          availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return availableFormat;
      }
    }
    return availableFormats[0];
  }

protected:
  /**
   * @brief Selects and initializes a suitable physical device (GPU) for Vulkan
   * operations.
   *
   * This pure virtual function is responsible for picking a physical device
   * from the available Vulkan-compatible devices on the system. Implementations
   * should evaluate the devices based on required features, performance, and
   * compatibility, and select the most appropriate one.
   *
   * @param instance The Vulkan instance used to enumerate physical devices.
   */
  virtual void pickPhysicalDevice(VkInstance instance) = 0;

  /**
   * @brief Creates the logical device for the rendering context.
   *
   * This pure virtual function is responsible for initializing and creating
   * the logical device, which is required for rendering operations. The
   * implementation should handle all necessary setup and resource allocation
   * for the logical device.
   *
   * @note Must be implemented by derived classes.
   */
  virtual void createLogicalDevice() = 0;

  /**
   * @brief Creates a command pool for the rendering context.
   *
   * This function initializes and allocates the necessary resources for a
   * command pool, which is used to manage the memory and lifecycle of command
   * buffers in the rendering pipeline. Derived classes should implement this
   * method to set up the command pool according to the specific graphics API or
   * rendering backend being used.
   *
   * @note Must be called before allocating or recording command buffers.
   */
  virtual void createCommandPool();

  /**
   * @brief Retrieves the maximum usable sample count for multisampling.
   *
   * This function queries the physical device capabilities and determines the
   * highest supported sample count that can be used for multisample
   * anti-aliasing (MSAA).
   *
   * @return VkSampleCountFlagBits The maximum supported sample count for MSAA.
   */
  VkSampleCountFlagBits getMaxUsableSampleCount() const;

  VkDevice _logicalDevice;          // Logical device for rendering operations
  VkPhysicalDevice _physicalDevice; // Physical device (GPU) used for rendering
  VkQueue _graphicsQueue; // Graphics queue for submitting rendering commands
  VkCommandPool _commandPool; // Command pool for managing command buffers
  VkSampleCountFlagBits _msaaSamples =
      VK_SAMPLE_COUNT_1_BIT; // MSAA sample count

  std::shared_ptr<VulkanContext>
      _vulkanContext; // Shared pointer to Vulkan context
};
} // namespace evan
