/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopBackend
*/

#pragma once

#include "EvanPlatform.hpp"

#include "ADeviceBackend.hpp"
#include "IDesktopPlatform.hpp"
#include "ASwapchainImage.hpp"

#include "Version.hpp"

/**
 * @brief List of Vulkan device extensions to enable.
 *
 * This vector contains the names of device extensions that will be
 * enabled for the Vulkan logical device. Device extensions provide
 * additional functionality beyond the core Vulkan specification. The
 * "VK_KHR_swapchain" extension is essential for presenting rendered
 * images to a display surface, while "VK_KHR_portability_subset" is
 * used to ensure compatibility across different platforms, particularly
 * on macOS where Vulkan support is provided through MoltenVK.
 */
const std::vector<const char *> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	"VK_KHR_portability_subset",
	VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME
};

namespace evan
{
	/**
	 * @brief Struct to encapsulate details about swap chain support.
	 *
	 * This struct contains information about the swap chain capabilities,
	 * available surface formats, and present modes for a Vulkan surface.
	 * It is typically used to query and store the swap chain support
	 * details for a Vulkan physical device.
	 *
	 * @struct SwapChainSupportDetails
	 */
	struct SwapChainSupportDetails {
		/*
		 * @brief Vulkan surface capabilities, such as the minimum and
		 * maximum image count, extent, and supported transforms.
		 */
		VkSurfaceCapabilitiesKHR capabilities;

		/*
		 * @brief A list of supported surface formats (color space and pixel
		 * format).
		 */
		std::vector<VkSurfaceFormatKHR> formats;

		/*
		 * @brief A list of supported presentation modes (e.g., FIFO,
		 * Mailbox, etc.).
		 */
		std::vector<VkPresentModeKHR> presentModes;
	};

	/**
	 * @brief Desktop backend implementation.
	 *
	 * This class provides an implementation of the backend for Desktop. It
	 * handles platform-specific functionality related to Desktop, such as
	 * creating a Vulkan surface for rendering or later handling input events.
	 */
	class DesktopBackend: public ADeviceBackend
	{
		public:
		/**
		 * @brief Constructor for DesktopBackend.
		 *
		 * @param platform The platform instance to be used for Desktop
		 * initialization.
		 */
		DesktopBackend(const IPlatform &platform);

		/**
		 * @brief Destructor for DesktopBackend.
		 *
		 * Ensures proper cleanup of Desktop resources.
		 */
		~DesktopBackend();

		/**
		 * @brief Preprocesses a frame before rendering.
		 *
		 * This function can be used to perform any necessary preprocessing
		 * steps before rendering a frame, such as updating uniform buffers,
		 * handling input events, or performing any other per-frame updates. In
		 * the case of the Desktop backend, no specific preprocessing is needed,
		 * so this function simply returns true.
		 *
		 * @param swapchainContext The swap chain context for the current frame.
		 *
		 * @return Always returns true, because no specific preprocessing is
		 * needed for the Desktop backend.
		 */
		bool preprocessFrame(ASwapchainContext &swapchainContext) override
		{
			// No specific preprocessing needed for Desktop backend
			return true;
		}

		/**
		 * @brief Processes a frame for rendering.
		 *
		 * This function is responsible for processing a frame before it is
		 * rendered. In the context of the Desktop backend, this function can be
		 * used to perform any necessary operations on the frame, such as
		 * updating uniform buffers, handling input events, or performing any
		 * other per-frame updates. However, in the current implementation, no
		 * specific processing is needed for the Desktop backend, so this
		 * function simply returns true.
		 *
		 * @param presentInfo The presentation information for the current
		 * frame, which may include details about the swap chain and
		 * presentation mode.
		 * @param swapchainImage The swap chain image that will be rendered to
		 * for the current frame. This parameter can be used to access the image
		 * and perform any necessary operations on it before rendering.
		 *
		 * @return true if the frame was processed successfully, false
		 * otherwise.
		 */
		bool processFrame(VkPresentInfoKHR presentInfo,
						  ASwapchainImage &swapchainImage) override;

		/**
		 * @brief Postprocesses a frame after rendering.
		 *
		 * This function can be used to perform any necessary postprocessing
		 * steps after rendering a frame, such as presenting the rendered image
		 * to the screen, handling synchronization, or performing any other
		 * operations that need to occur after rendering. In the case of the
		 * Desktop backend, no specific postprocessing is needed, so this
		 * function simply returns true.
		 *
		 * @param swapchainContext The swap chain context for the current frame,
		 * which may include information about the swap chain and presentation
		 * details.
		 *
		 * @return Always returns true, because no specific postprocessing is
		 * needed for the Desktop backend.
		 */
		bool postprocessFrame(ASwapchainContext &swapchainContext) override
		{
			// No specific postprocessing needed for Desktop backend
			return true;
		}

		/**
		 * @brief Counts the number of supported swapchain formats for the
		 * Vulkan surface.
		 *
		 * This function queries the Vulkan API to determine the number of
		 * supported swapchain formats for the surface associated with the
		 * Desktop backend. It retrieves the list of supported surface formats
		 * and returns the count, which can be used to determine how many
		 * different formats are available for use when creating a swapchain.
		 *
		 * @return The number of supported swapchain formats for the Vulkan
		 * surface.
		 */
		uint32_t countSwapchainFormats() const override;

		/**
		 * @brief Enumerates the supported swapchain formats for the Vulkan
		 * surface.
		 *
		 * This function retrieves the list of supported swapchain formats for
		 * the surface associated with the Desktop backend and returns them as a
		 * vector of integers. Each integer in the vector represents a specific
		 * Vulkan format that is supported for use in the swapchain. This
		 * information can be used when creating a swapchain to select an
		 * appropriate format for rendering.
		 *
		 * @param swapchainFormatCount The number of swapchain formats to
		 * enumerate, which should match the count returned by
		 * countSwapchainFormats().
		 *
		 * @return A vector of integers representing the supported swapchain
		 * formats for the Vulkan surface.
		 */
		std::vector<int64_t> enumerateSwapchainFormats(
			uint32_t swapchainFormatCount) const override;

		/**
		 * @brief Finds the queue family indices for the Vulkan physical device.
		 *
		 * This function queries the Vulkan API to find the indices of the queue
		 * families that support graphics and presentation operations for the
		 * physical device associated with the Desktop backend. It returns a
		 * QueueFamilyIndices structure containing the indices of the graphics
		 * and presentation queue families, which can be used during device
		 * creation to ensure that the necessary queues are available for
		 * rendering and presentation.
		 *
		 * @return A QueueFamilyIndices structure containing the indices of the
		 * graphics and presentation queue families for the Vulkan physical
		 * device.
		 *
		 * @note This function is an override of the pure virtual function
		 * defined in the ADeviceBackend class, and it provides the specific
		 * implementation for finding queue family indices in the context of the
		 * Desktop backend.
		 */
		QueueFamilyIndices findQueueFamilies() override;

		/**
		 * @brief Queries the swap chain support details for a given Vulkan
		 * physical device and surface.
		 * This function retrieves information about the swap chain
		 * capabilities, available surface formats, and present modes for the
		 * specified Vulkan physical device and surface. It populates a
		 * SwapChainSupportDetails structure with the retrieved information.
		 *
		 * @return A SwapChainSupportDetails structure containing the swap chain
		 * support details for the specified device and surface.
		 */
		evan::SwapChainSupportDetails
			querySwapChainSupport();

		/**
		 * @brief Creates the presentation queue for the device context.
		 *
		 * This function retrieves the presentation queue from the logical
		 * device and stores it in a member variable (not shown in this
		 * snippet). It assumes that the logical device has already been created
		 * and that the presentation queue family index has been determined.
		 */
		void createPresentQueue();

		/**
		 * The Vulkan surface associated with the Desktop backend, used for
		 * rendering and presentation. This member variable holds the handle to
		 * the Vulkan surface that is created for the Desktop backend, allowing
		 * it to be used in various Vulkan operations such as creating a
		 * swapchain or presenting rendered images to the screen.
		 */
		VkSurfaceKHR _surface;

		/**
		 * The Vulkan presentation queue associated with the Desktop backend,
		 * used for presenting rendered images to the screen. This member
		 * variable holds the handle to the Vulkan queue that is responsible for
		 * presenting images from the swapchain to the display surface. It is
		 * created based on the queue family indices determined during device
		 * selection and is essential for the presentation process in Vulkan.
		 */
		VkQueue _presentQueue;

		protected:
		/**
		 * @brief Creates a Vulkan instance and initializes the device backend
		 * for the Desktop platform.
		 *
		 * This function is responsible for creating a Vulkan instance and
		 * performing any necessary initialization for the device backend
		 * specific to the Desktop platform. It takes the platform information,
		 * application name, and application version as parameters to set up the
		 * Vulkan instance with the appropriate application info and extensions
		 * required for the Desktop backend.
		 *
		 * @param platform The platform instance containing information about
		 * the Desktop environment.
		 * @param appName The name of the application, used for Vulkan instance
		 * creation.
		 * @param appVersion The version of the application, used for Vulkan
		 * instance creation.
		 *
		 * @note This function is an override of the pure virtual function
		 * defined in the ADeviceBackend class, and it provides the specific
		 * implementation for creating a Vulkan instance and initializing the
		 * device backend in the context of the Desktop platform.
		 */
		void createInstance(const IPlatform &platform,
							const std::string &appName,
							Version &appVersion) override;

		/**
		 * @brief Creates a Vulkan logical device and retrieves the necessary
		 * queues for the Desktop platform.
		 *
		 * This function is responsible for creating a Vulkan logical device
		 * based on the selected physical device and retrieving the necessary
		 * queues (such as graphics and presentation queues) for the Desktop
		 * platform. It uses the queue family indices determined during device
		 * selection to ensure that the created logical device has access to the
		 * required queues for rendering and presentation operations.
		 *
		 * @note This function is an override of the pure virtual function
		 * defined in the ADeviceBackend class, and it provides the specific
		 * implementation for creating a Vulkan logical device and retrieving
		 * queues in the context of the Desktop platform.
		 */
		void createLogicalDevice() override;

		/**
		 * @brief Picks a suitable Vulkan physical device for the Desktop
		 * platform.
		 *
		 * This function is responsible for selecting a Vulkan physical device
		 * that is suitable for use with the Desktop platform. It evaluates the
		 * available physical devices and checks their capabilities, features,
		 * and support for required extensions to determine which device is best
		 * suited for the application's needs. The selected physical device will
		 * be used for creating the logical device and performing rendering
		 * operations.
		 *
		 * @note This function is an override of the pure virtual function
		 * defined in the ADeviceBackend class, and it provides the specific
		 * implementation for picking a Vulkan physical device in the context of
		 * the Desktop platform.
		 */
		void pickPhysicalDevice() override;

		private:
		/**
		 * @brief Checks if the specified Vulkan physical device is suitable for
		 * use.
		 *
		 * This function evaluates the capabilities and features of the given
		 * Vulkan physical device to determine if it meets the requirements for
		 * the application. It checks for support for necessary queue families,
		 * device extensions, and swap chain support.
		 *
		 * @param device The Vulkan physical device to evaluate.
		 * @param surface The Vulkan surface to check for compatibility with the
		 * device.
		 * @param deviceExtensions A list of required device extensions that
		 * must be supported by the device.
		 * @return true if the device is suitable for use, false otherwise.
		 */
		bool isDeviceSuitable(VkPhysicalDevice physicalDevice,
			std::vector<const char *> deviceExtensions);

		/**
		 * @brief Checks if the specified Vulkan physical device supports the
		 * required device extensions. This function queries the Vulkan API to
		 * determine if the given physical device supports all of the required
		 * device extensions specified in the deviceExtensions vector.
		 * @param device The Vulkan physical device to check for extension
		 * support.
		 * @param deviceExtensions A list of required device extensions that
		 * must be supported by the device.
		 * @return true if the device supports all required extensions, false
		 * otherwise.
		 */
		bool checkDeviceExtensionSupport(
			VkPhysicalDevice device,
			std::vector<const char *> deviceExtensions);

		/**
		 * @brief Checks if the required validation layers are supported.
		 *
		 * @return true if all requested validation layers are supported, false
		 * otherwise.
		 */
		bool checkValidationLayerSupport();

		/**
		 * @brief Populates the debug messenger create info structure.
		 *
		 * @param createInfo The structure to be populated with debug messenger
		 * information.
		 * @param debugCallback The callback function to be used for debug
		 * messages.
		 */
		void populateDebugMessengerCreateInfo(
			VkDebugUtilsMessengerCreateInfoEXT &createInfo,
			debugCallback_t debugCallback);
	};
}	 // namespace evan
