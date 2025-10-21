/*
** ETIB PROJECT, 2025
** evan
** File description:
** GraphicalContext
*/

#pragma once

#include <iostream>

#include "AGraphicalContext.hpp"

#include "vk/RenderingContext.hpp"
#include "vk/SwapchainContext.hpp"

#include "Version.hpp"

namespace evan {
namespace vk {
/**
 * @brief List of Vulkan validation layers to enable.
 *
 * This vector contains the names of validation layers that will be enabled for
 * the Vulkan instance. Validation layers are used to assist in debugging and
 * development by providing detailed error checking and reporting for Vulkan API
 * usage. The "VK_LAYER_KHRONOS_validation" layer is the standard validation
 * layer provided by the Khronos Group.
 */
const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

/**
 * @brief List of Vulkan device extensions to enable.
 *
 * This vector contains the names of device extensions that will be enabled for
 * the Vulkan logical device. Device extensions provide additional functionality
 * beyond the core Vulkan specification. The "VK_KHR_swapchain" extension is
 * essential for presenting rendered images to a display surface, while
 * "VK_KHR_portability_subset" is used to ensure compatibility across different
 * platforms, particularly on macOS where Vulkan support is provided through
 * MoltenVK.
 */
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset"};

#ifdef NDEBUG
/**
 * @brief Flag to enable or disable Vulkan validation layers.
 *
 * When set to true, validation layers are enabled, providing additional
 * debugging and error-checking capabilities during development. This is useful
 * for catching potential issues in Vulkan API usage. In release builds (when
 * NDEBUG is defined), validation layers are typically disabled to improve
 * performance.
 */
const bool enableValidationLayers = false;
#else
/**
 * @brief Flag to enable or disable Vulkan validation layers.
 *
 * When set to true, validation layers are enabled, providing additional
 * debugging and error-checking capabilities during development. This is useful
 * for catching potential issues in Vulkan API usage. In release builds (when
 * NDEBUG is defined), validation layers are typically disabled to improve
 * performance.
 */
const bool enableValidationLayers = true;
#endif

/**
 * @brief Class representing a Vulkan graphical context.
 *
 * This class is responsible for managing the Vulkan instance, including
 * creating the instance, handling validation layers, and providing access
 * to Vulkan-specific resources. It inherits from the AGraphicalContext
 * abstract class, which defines the interface for graphical contexts.
 * The GraphicalContext class encapsulates Vulkan-specific functionality
 * and integrates with the rendering and swapchain contexts.
 */
class GraphicalContext : public evan::AGraphicalContext {
public:
  /*
  ** @brief Callback type for the debug messenger.
  **
  ** This callback is used to handle debug messages from the Vulkan API.
  ** It is used to log the debug messages from the Vulkan API.
  **
  ** @param severity the severity of the message
  ** @param type the type of the message
  ** @param pCallbackData the callback data
  ** @param pUserData the user data
  **
  ** @return true if the message is handled, false otherwise
  */
  typedef VkBool32 debugCallback_t(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                   VkDebugUtilsMessageTypeFlagsEXT,
                                   const VkDebugUtilsMessengerCallbackDataEXT *,
                                   void *);

  /**
   * @brief Constructs a new GraphicalContext object.
   *
   * This constructor initializes a new GraphicalContext object with default
   * application and engine names and versions. It creates a Vulkan instance,
   * sets up the rendering context with default window properties, and
   * initializes the swapchain context using properties from the Vulkan context.
   *
   * @note The default window size is set to 800x600 pixels.
   * @note The default application name is "Hello, World !" and the engine name
   * is "Evan".
   * @note The default application and engine versions are set to 1.0.0
   */
  GraphicalContext();

  /**
   * @brief Constructs a new GraphicalContext object with specified parameters.
   *
   * This constructor initializes a new GraphicalContext object with the
   * provided application name, application version, engine name, engine
   * version, and window dimensions. It creates a Vulkan instance, sets up the
   * rendering context with the specified window properties, and initializes the
   * swapchain context using properties from the Vulkan context.
   *
   * @param appName The name of the application.
   * @param appVersion The version of the application as a Version object.
   * @param engineName The name of the engine.
   * @param engineVersion The version of the engine as a Version object.
   * @param windowWidth The width of the window (default is 800).
   * @param windowHeight The height of the window (default is 600).
   */
  GraphicalContext(const std::string &appName, const Version &appVersion,
                   const std::string &engineName, const Version &engineVersion,
                   unsigned int windowWidth = 800,
                   unsigned int windowHeight = 600);

  /**
   * @brief Deleted copy constructor to prevent copying of GraphicalContext
   * instances.
   */
  GraphicalContext(const GraphicalContext &other) = delete; // Copy constructor

  // Destructor
  ~GraphicalContext();

  /**
   * @brief Retrieves the required Vulkan instance extensions.
   *
   * This function returns a list of Vulkan instance extensions that are
   * required for the graphical context. The extensions are necessary for
   * creating a Vulkan instance that is compatible with the rendering context
   * and any additional features such as validation layers.
   *
   * @return A vector of strings representing the names of the required Vulkan
   * instance extensions.
   */
  std::vector<std::string> getInstanceExtensions() override;

  /**
   * @brief Creates the Vulkan instance.
   *
   * This function creates a Vulkan instance with the required extensions and
   * validation layers.
   *
   * @note Overrides the pure virtual function from AGraphicalContext.
   */
  void createInstance() override;

private:
  //////////////////
  /// Properties ///
  //////////////////
  std::string _appName;    // Application name
  Version *_appVersion;    // Application version (major, minor, patch)
  std::string _engineName; // Engine name
  Version *_engineVersion; // Engine version (major, minor, patch)

  /**
   * @brief Checks if the requested validation layers are supported.
   *
   * This function queries the available validation layers on the system and
   * verifies that all requested layers are present. It is used to ensure that
   * the validation
   */
  bool checkValidationLayerSupport();

  /**
   * @brief Populates a VkDebugUtilsMessengerCreateInfoEXT structure with the
   * specified debug callback.
   *
   * This function initializes the provided VkDebugUtilsMessengerCreateInfoEXT
   * structure with the necessary parameters to set up Vulkan debug messaging.
   * The debug callback function pointer is assigned to the structure, enabling
   * custom handling of Vulkan validation layer messages.
   *
   * @param createInfo Reference to the VkDebugUtilsMessengerCreateInfoEXT
   * structure to populate.
   * @param debugCallback The callback function to be used for debug message
   * handling.
   */
  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo,
      debugCallback_t debugCallback);
};
} // namespace vk
} // namespace evan
