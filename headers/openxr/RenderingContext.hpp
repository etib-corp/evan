/*
** ETIB PROJECT, 2025
** evan
** File description:
** RenderingContext
*/

#pragma once

#include "ARenderingContext.hpp"
#include "Utils.hpp"
#include "openxr/Openxr-include.hpp"

#include "Utils.hpp"

namespace evan {
namespace openxr {

/**
 * @brief Properties structure for OpenXR rendering context.
 *
 * This structure encapsulates the essential properties required to initialize
 * and manage an OpenXR (Extended Reality) rendering context. It includes handles to
 * the OpenXR instance, the OpenXR system identifier, and the Vulkan instance used for
 * rendering.
 *
 */
struct RenderingContextPropertiesXR {
  XrInstance _XRinstance;     // Handle to the OpenXR instance
  XrSystemId _XRsystemID;     // Identifier for the OpenXR system
  VkInstance _vulkanInstance; // Vulkan instance associated with the OpenXR session
};

/**
 * @brief OpenXR Rendering Context class.
 *
 * This class manages the rendering context for OpenXR applications, integrating
 * with OpenXR and Vulkan. It extends the base ARenderingContext class to
 * provide OpenXR-specific functionality, including physical device selection and
 * logical device creation tailored for OpenXR rendering.
 *
 */
class RenderingContext : public evan::ARenderingContext {
public:
  /**
   * @brief Constructs a RenderingContext object with the specified properties.
   *
   * Initializes the OpenXR rendering context using the provided properties, which
   * include the OpenXR instance, OpenXR system ID, and Vulkan instance. This
   * constructor also calls the internal initialization routine.
   *
   * @param properties The properties required to initialize the OpenXR rendering
   * context, including OpenXR instance, OpenXR system ID, and Vulkan instance.
   */
  RenderingContext(const RenderingContextPropertiesXR &properties);

  // Destructor
  ~RenderingContext() override;

protected:
  /**
   * @brief Selects and initializes the Vulkan physical device compatible with
   * the current OpenXR instance and system.
   *
   * This function retrieves the Vulkan physical device associated with the
   * OpenXR system by using the xrGetVulkanGraphicsDevice2KHR extension
   * function. It ensures that the OpenXR instance is initialized and that a
   * physical device has not already been selected. If the required extension
   * function cannot be loaded or the physical device cannot be retrieved, error
   * messages are printed to std::cerr.
   *
   * @param instance The Vulkan instance to be used for device selection.
   *
   * @note The selected physical device is stored in the member variable
   * _physicalDevice.
   */
  void pickPhysicalDevice(VkInstance instance) override;

  /**
   * @brief Creates a Vulkan logical device for the current physical device
   * using OpenXR integration.
   *
   * This function initializes the Vulkan logical device required for rendering
   * in an OpenXR context. It first checks if the physical device is initialized and
   * if a logical device has already been created. It retrieves the OpenXR
   * function pointer for xrCreateVulkanDeviceKHR, sets up the necessary Vulkan
   * device and queue creation structures, and invokes the OpenXR function to
   * create the Vulkan logical device. Upon success, it retrieves the graphics
   * queue from the created logical device.
   *
   * @note The logical device and graphics queue are stored in member variables
   * _logicalDevice and _graphicsQueue.
   */
  void createLogicalDevice() override;

private:
  XrInstance _XRinstance = XR_NULL_HANDLE; // The OpenXR instance used for rendering
  XrSystemId _XRsystemID =
      XR_NULL_SYSTEM_ID; // The OpenXR system ID for the rendering context
  VkInstance _vulkanInstance =
      VK_NULL_HANDLE; // The Vulkan instance associated with the OpenXR session
};
} // namespace openxr
} // namespace evan
