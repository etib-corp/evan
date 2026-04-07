/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrBackend
*/

#pragma once

#include "openxr/IXrPlatform.hpp"
#include "openxr/InteractionProfile.hpp"

#include "ADeviceBackend.hpp"

#include <cstring>
#include <iostream>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <vector>

namespace evan
{
	/**
	 * @brief The XrDeviceBackend class is responsible for managing the OpenXR
	 * device backend, including instance creation, session management, and
	 * frame processing.
	 *
	 * This class encapsulates the functionality required to interface with the
	 * OpenXR runtime, including creating an instance, picking a physical
	 * device, creating a logical device, and managing the rendering loop for XR
	 * applications. It also handles platform-specific configurations and
	 * extensions required for OpenXR on different platforms.
	 */
	class XrDeviceBackend: public ADeviceBackend
	{
		public:
		/**
		 * @brief Constructs an XrDeviceBackend instance with the specified
		 * platform.
		 *
		 * Initializes the OpenXR instance and retrieves the system ID for the
		 * XR device. It also sets up the necessary resources for managing the
		 * XR session and rendering loop.
		 *
		 * @param platform The platform interface providing required extensions
		 * and configurations for OpenXR instance creation.
		 */
		XrDeviceBackend(const IPlatform &platform);

		~XrDeviceBackend() override;

		/**
		 * @brief Retrieves the system ID for the OpenXR device.
		 *
		 * This method queries the OpenXR runtime to obtain the system ID for
		 * the XR device that the application will target. The system ID is
		 * essential for creating sessions and managing the XR experience. The
		 * retrieved system ID is stored in the member variable _systemId for
		 * later use in session creation and rendering.
		 */
		bool preprocessFrame(ASwapchainContext &swapchainContext) override;

		/**
		 * @brief Processes a frame for the OpenXR session.
		 *
		 * This method handles the processing of a single frame in the OpenXR
		 * session, including acquiring the next available swapchain image,
		 * submitting rendering commands, and presenting the rendered frame to
		 * the XR device. It takes a VkPresentInfoKHR structure containing
		 * presentation information and an ASwapchainImage reference for
		 * managing the swapchain image during presentation.
		 */
		bool processFrame(VkPresentInfoKHR presentInfo,
						  ASwapchainImage &swapchainImage) override;

		/**
		 * @brief Post-processes a frame after presentation in the OpenXR
		 * session.
		 *
		 * This method performs any necessary post-processing after a frame has
		 * been presented to the XR device. It may involve updating the
		 * predicted display time for the next frame, handling any
		 * synchronization or cleanup tasks, and preparing for the next
		 * rendering cycle in the XR session.
		 */
		bool postprocessFrame(ASwapchainContext &swapchainContext) override;

		/**
		 * @brief Finds the queue family indices for graphics and presentation
		 * operations.
		 *
		 * This method queries the OpenXR runtime to determine the queue family
		 * indices that support graphics and presentation operations. It returns
		 * a QueueFamilyIndices structure containing the indices of the graphics
		 * and presentation queue families, if they are found. If the required
		 * queue families are not found, the corresponding optional values in
		 * the QueueFamilyIndices structure will be empty.
		 *
		 * @return QueueFamilyIndices A structure containing the indices of the
		 * graphics and presentation queue families, if available.
		 */
		QueueFamilyIndices findQueueFamilies() override;

		/**
		 * @brief Count the number of available swapchain formats for the Vulkan
		 * device.
		 *
		 * This method queries the OpenXR runtime to determine how many
		 * swapchain formats are supported by the physical device. The count of
		 * swapchain formats is essential for creating a swapchain that is
		 * compatible with the device's capabilities.
		 *
		 * @return uint32_t The number of available swapchain formats supported
		 * by the Vulkan device.
		 */
		uint32_t countSwapchainFormats() const override;

		/**
		 * @brief Enumerates the available swapchain formats for the Vulkan
		 * device.
		 *
		 * This method retrieves the list of supported swapchain formats from
		 * the OpenXR runtime. It takes the count of swapchain formats as a
		 * parameter and returns a vector of int64_t values representing the
		 * available formats. These formats are crucial for configuring the
		 * swapchain to ensure compatibility with the device's capabilities.
		 *
		 * @param swapchainFormatCount The number of swapchain formats to
		 * enumerate, typically obtained from countSwapchainFormats().
		 *
		 * @return std::vector<int64_t> A vector containing the enumerated
		 * swapchain formats supported by the Vulkan device.
		 */
		std::vector<int64_t> enumerateSwapchainFormats(
			uint32_t swapchainFormatCount) const override;

		/**
		 * @brief Enumerates the available view configurations for the OpenXR
		 * system.
		 *
		 * This method queries the OpenXR runtime to retrieve the list of
		 * supported view configurations for the XR system. It returns a vector
		 * of XrViewConfigurationView structures, each containing information
		 * about a specific view configuration, such as recommended image rect
		 * dimensions and swapchain sample counts. This information is essential
		 * for setting up the rendering pipeline to match the capabilities of
		 * the XR device.
		 *
		 * @return std::vector<XrViewConfigurationView> A vector containing the
		 * enumerated view configurations supported by the OpenXR system. Each
		 * entry in the vector provides details about a specific view
		 * configuration, which can be used to configure the rendering pipeline
		 * accordingly.
		 */
		std::vector<XrViewConfigurationView>
			enumerateViewConfigurations() const;

		/**
		 * The OpenXR instance, which represents the
		 * connection between the application and the
		 * OpenXR runtime. It is used to create and manage
		 * OpenXR resources and to query the capabilities
		 * of the OpenXR implementation.
		 */
		XrInstance _XrInstance = XR_NULL_HANDLE;

		/**
		 * The OpenXR system ID, which represents the
		 * specific XR system (e.g., VR headset) that
		 * the application is targeting. It is used to
		 * query the capabilities of the XR system and
		 * to create sessions for rendering.
		 */
		XrSystemId _systemId = XR_NULL_SYSTEM_ID;

		/**
		 * The OpenXR session, which represents an active
		 * connection to the XR system. It is used to manage
		 * the lifecycle of the XR experience and to submit
		 * rendering commands for display on the XR device.
		 */
		XrSession _session = XR_NULL_HANDLE;

		/**
		 * The OpenXR space, which defines a coordinate system
		 * for the application. It is used to track the
		 * position and orientation of the XR device and to
		 * render content in the correct location within the
		 * XR environment.
		 */
		XrSpace _space = XR_NULL_HANDLE;

		InteractionProfile _interactionProfile;

		/**
		 * Flag to track if the session is currently running.
		 *
		 * This is used to determine if the application should continue running
		 * or if it should exit based on the session state changes received from
		 * the OpenXR runtime.
		 */
		bool _sessionRunning = false;

		protected:
		/**
		 * @brief Creates an OpenXR instance with the specified application name
		 * and version.
		 *
		 * This method initializes the OpenXR instance by configuring the
		 * necessary parameters, including the application name and version. It
		 * also retrieves the required instance extensions from the platform and
		 * includes them in the instance creation process. The created instance
		 * is stored in the member variable _XrInstance for later use in
		 * managing the XR session and rendering.
		 *
		 * @param platform The platform interface providing required extensions
		 * and configurations for OpenXR instance creation.
		 * @param appName The name of the application, used for identifying the
		 * application to the OpenXR runtime.
		 * @param appVersion The version of the application, used for
		 * identifying the application to the
		 */
		void createInstance(const IPlatform &platform,
							const std::string &appName,
							Version &appVersion) override;

		/**
		 * @brief Creates a logical device for the OpenXR session.
		 *
		 * This method sets up the necessary resources and configurations to
		 * create a logical device that can be used for rendering and
		 * interacting with the OpenXR session. It involves selecting
		 * appropriate queue families, configuring device features, and creating
		 * the logical device using the OpenXR runtime. The created logical
		 * device is essential for submitting rendering commands and managing
		 * resources within the XR environment.
		 *
		 * @note The implementation of this method may involve platform-specific
		 * configurations and may require additional parameters depending on the
		 * requirements of the OpenXR runtime and the capabilities of the
		 * underlying hardware.
		 */
		void createLogicalDevice() override;

		/**
		 * @brief Picks a physical device that is compatible with the OpenXR
		 * session.
		 *
		 * This method queries the OpenXR runtime to identify and select a
		 * physical device (e.g., GPU) that is compatible with the requirements
		 * of the XR session. It evaluates the available devices based on their
		 * capabilities, such as support for required features and extensions,
		 * and selects the most suitable device for rendering and interaction
		 * within the XR environment. The selected physical device is essential
		 * for creating a logical device and managing resources for the XR
		 * session.
		 *
		 * @note The implementation of this method may involve platform-specific
		 * considerations and may require additional parameters to evaluate the
		 * capabilities of the available devices effectively.
		 */
		void pickPhysicalDevice() override;

		/**
		 * @brief Creates a visualized space for the XR session.
		 *
		 * This method sets up a reference space that can be used for rendering
		 * and tracking within the XR environment. It defines the coordinate
		 * system for the application and allows for accurate tracking of the XR
		 * device's position and orientation in the virtual space.
		 *
		 * @note This function should be called after creating the OpenXR
		 * session and before starting the rendering loop to ensure that the
		 * application has a valid space for rendering and tracking.
		 */
		void createVisualizedSpace();

		/**
		 * Retrieves the list of Vulkan instance extensions available for the
		 * validation layer.
		 *
		 * This function enumerates all instance extension properties supported
		 * by the VK_LAYER_KHRONOS_validation layer. It performs two queries:
		 * first to determine the number of available extensions, then to
		 * retrieve the full extension list.
		 *
		 * @return A vector of VkExtensionProperties containing all available
		 * instance extensions. Returns an empty vector if enumeration fails at
		 * any step.
		 *
		 * @note The function queries extensions specifically for the
		 * "VK_LAYER_KHRONOS_validation" layer.
		 */
		std::vector<VkExtensionProperties> getInstanceExtensions() const;

		/**
		 * The predicted display time for the next frame, used for
		 * synchronizing rendering with the XR device's display
		 * refresh cycle. It is updated each frame to ensure
		 * smooth and responsive rendering in the XR environment.
		 */
		XrTime _predictedDisplayTime = 0;

		private:
		/**
		 * @brief Creates an OpenXR instance with platform-specific extensions
		 * and configurations.
		 *
		 * Initializes the XrInstance by:
		 * - Collecting required instance extensions from the platform
		 * - Configuring XrInstanceCreateInfo with the extensions
		 * - Setting platform-specific creation parameters via Android-specific
		 * info
		 * - Creating the XR instance and logging any errors
		 *
		 * @param platform Reference to the IPlatform implementation providing
		 * required extensions and platform-specific OpenXR instance creation
		 * information
		 *
		 * @note The function uses dynamic_cast which may return nullptr.
		 * Consider adding null pointer checks before dereferencing the casted
		 * pointer.
		 *
		 * @see xrCreateInstance
		 * @see XrInstanceCreateInfo
		 */
		void createXrInstance(const IPlatform &platform);

		/**
		 * @brief Retrieves the OpenXR system ID for the head-mounted display
		 * form factor.
		 *
		 * Initializes the system information structure with the appropriate
		 * form factor (XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY) and queries the
		 * OpenXR instance for a compatible system. The retrieved system ID is
		 * stored in the member variable _systemId for later use.
		 *
		 * @see xrGetSystem
		 * @see XrSystemGetInfo
		 */
		void getSystem();

		/**
		 * @brief Creates an OpenXR session for the selected system.
		 *
		 * This method initializes the session creation information structure
		 * with the selected system ID and any required graphics binding
		 * information. It then creates the session using the OpenXR runtime and
		 * stores the session handle in the member variable _session for
		 * managing the XR experience.
		 */
		void createSession();

		/**
		 * @brief Retrieves the required Vulkan instance extensions for OpenXR.
		 *
		 * Queries available instance extensions and filters them to return only
		 * those that are required for debugging and validation purposes.
		 * Specifically, this method identifies and returns the following
		 * extensions if available:
		 * - VK_EXT_DEBUG_UTILS_EXTENSION_NAME: For debug utility functions
		 * - VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME: For validation layer
		 * features
		 *
		 * @return std::vector<const char*> A vector of C-strings containing the
		 * names of required instance extensions that are available on the
		 * current system. The vector may be empty if none of the required
		 * extensions are available.
		 *
		 * @note The returned pointers are valid only while the extension data
		 * remains in memory. Callers should not assume the pointers remain
		 * valid after subsequent calls to getInstanceExtensions().
		 */
		std::vector<const char *> getRequiredInstanceExtensions() const;

		/**
		 * Retrieves the required instance extensions for Android in the OpenXR
		 * device backend.
		 *
		 * This function filters and returns a list of available Vulkan layers
		 * that match the KHRONOS validation layer. It is specifically used to
		 * configure the OpenXR instance with the appropriate validation layers
		 * on Android platforms.
		 *
		 */
		std::vector<const char *> getRequiredInstanceExtensionsAndroid();
	};
}	 // namespace evan