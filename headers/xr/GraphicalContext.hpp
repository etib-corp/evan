/*
** ETIB PROJECT, 2025
** maverik
** File description:
** GraphicalContext
*/

#pragma once

#include "AGraphicalContext.hpp"
#include "xr/Openxr-include.hpp"
#include "xr/RenderingContext.hpp"
#include "xr/SwapchainContext.hpp"

namespace maverik {
namespace xr {

/**
 * @struct GraphicalContextProperties
 * @brief Represents the properties required for initializing a graphical
 * context in an XR environment.
 *
 * This structure contains essential information for setting up and managing a
 * graphical context, including the XR instance, XR system ID, and a shared
 * rendering context.
 */
struct GraphicalContextPropertiesXR {
  XrInstance _XRinstance; // The XR instance used for rendering
  XrSystemId _XRsystemID; // The XR system ID
};

/**
 * @class GraphicalContext
 * @brief A class to manage the graphical context for XR rendering.
 *
 * This class is responsible for creating and managing the graphical context
 * required for rendering in an XR environment. It handles the creation of the
 * Vulkan instance, initialization of the XR session, and management of
 * visualized reference spaces.
 */
class GraphicalContext : public maverik::AGraphicalContext {
public:
  /**
   * @brief Constructs a GraphicalContext object for XR rendering.
   *
   * Initializes the graphical context using the provided XR instance and system
   * ID. This constructor creates the necessary XR and Vulkan instances, sets up
   * the rendering context, initializes the XR session, and prepares the
   * swapchain context for rendering.
   *
   * @param properties The properties required to initialize the XR graphical
   * context, including XR instance and system ID.
   *
   * @note This constructor will allocate and initialize internal resources
   * required for XR rendering. It is expected that the provided properties are
   * valid and compatible with the underlying XR runtime.
   */
  GraphicalContext(const GraphicalContextPropertiesXR &properties);

  // Destructor
  ~GraphicalContext();

  /**
   * @brief Retrieves the list of required XR instance extension names for this
   * graphical context.
   *
   * This function returns a vector containing the names of XR instance
   * extensions that are necessary for enabling Vulkan support in the XR
   * runtime.
   *
   * @return std::vector<std::string> A vector of required XR instance extension
   * names.
   *
   * @note Overrides the pure virtual function from AGraphicalContext.
   */
  std::vector<std::string> getInstanceExtensions() override;

protected:
  /**
   * @brief Creates a Vulkan instance compatible with the current OpenXR
   * runtime.
   *
   * This function queries the OpenXR runtime for the required Vulkan graphics
   * requirements and retrieves the necessary Vulkan-related function pointers.
   * It then prepares the Vulkan application and instance creation structures,
   * and uses the OpenXR extension function xrCreateVulkanInstanceKHR to create
   * a Vulkan instance suitable for use with the OpenXR system.
   *
   *
   * @note This function assumes that _XRinstance and _XRsystemID are valid and
   * initialized.
   * @note The created Vulkan instance is stored in the member variable
   * _instance.
   * @note Overrides the pure virtual function from AGraphicalContext.
   */
  void createInstance() override;

  /**
   * @brief Initializes the XR session for the graphical context.
   *
   * This function sets up the XR session using Vulkan as the graphics API.
   * It first checks if a session already exists, and if not, retrieves the
   * Vulkan context from the rendering context. It then prepares the necessary
   * graphics binding structure and session creation info, and attempts to
   * create the XR session.
   *
   *
   * @note This function assumes that _XRinstance and _XRsystemID are valid and
   * initialized.
   */
  void initializeSession();

  /**
   * @brief Creates a new visualized reference space for XR rendering.
   *
   * This function initializes an XrReferenceSpaceCreateInfo structure with the
   * type set to XR_REFERENCE_SPACE_TYPE_STAGE and a default pose. It then
   * attempts to create a new reference space using xrCreateReferenceSpace. If
   * successful, the created space is added to the _XRvisualizedSpaces
   * container. If the creation fails, an error message is printed to stderr.
   *
   * @note The function assumes that _XRsession is a valid XR session handle.
   */
  void createVisualizedSpace();

private:
  XrInstance _XRinstance = XR_NULL_HANDLE; // The XR instance used for rendering
  XrSystemId _XRsystemID = XR_NULL_SYSTEM_ID; // The XR system ID
  XrSession _XRsession = XR_NULL_HANDLE;      // The XR session handle

  std::vector<XrSpace>
      _XRvisualizedSpaces; // Container for visualized reference spaces
};
} // namespace xr
} // namespace maverik
