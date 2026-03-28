/*
** EPITECH PROJECT, 2026
** evan
** File description:
** ASwapchainContext
*/

#pragma once

#include "EvanPlatform.hpp"
#include "ADeviceBackend.hpp"
#include "ASwapchainImage.hpp"

#include <utility/math/matrix.hpp>

#include <map>
#include <memory>
#include <iostream>

namespace evan
{

	/**
	 * @class ASwapchainContext
	 * @brief Represents the context for managing the swapchain in the rendering
	 * system.
	 *
	 * The ASwapchainContext class encapsulates the functionality related to
	 * managing the swapchain in the rendering system. It is responsible for
	 * handling the creation, configuration, and management of the swapchain,
	 * which is a series of images used for presenting rendered frames to the
	 * display. This class serves as a central point for managing
	 * swapchain-related operations in the engine.
	 *
	 */
	class ASwapchainContext
	{
		public:
		virtual ~ASwapchainContext() = default;

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
		 * @brief Destroys the swapchain and releases associated resources.
		 *
		 * This pure virtual function must be implemented by derived classes to
		 * handle the destruction of the swapchain and the release of any
		 * resources associated with it. The function takes a Vulkan device as a
		 * parameter, which is used to perform the necessary cleanup operations.
		 * Implement this function to ensure that all resources related to the
		 * swapchain are properly released when the swapchain is no longer
		 * needed.
		 *
		 * @param device The Vulkan device used to perform cleanup operations
		 * for the swapchain.
		 *
		 * @note This function is pure virtual and must be implemented by
		 * derived classes to handle the specific cleanup logic for the
		 * swapchain in the context of the rendering system.
		 */
		virtual void destroy(VkDevice device) = 0;

		/**
		 * @brief Retrieves the render pass associated with the swapchain
		 * context.
		 *
		 * This function returns the Vulkan render pass that is associated with
		 * the swapchain context. The render pass defines the structure of the
		 * rendering operations, including the attachments, subpasses, and
		 * dependencies. It is essential for configuring the graphics pipeline
		 * and ensuring that rendering operations are performed correctly.
		 * Implement this function to provide access to the render pass used in
		 * the swapchain context.
		 *
		 * @return The Vulkan render pass associated with the swapchain context.
		 */
		VkRenderPass getRenderPass() const;

		/**
		 * @brief Acquires the next available image from the swapchain for
		 * rendering.
		 *
		 * This pure virtual function must be implemented by derived classes to
		 * handle the acquisition of the next available image from the swapchain
		 * for rendering. The function takes several parameters, including the
		 * index of the image to acquire, the Vulkan device, a semaphore to
		 * signal when the image is available, a fence to signal when the image
		 * is in flight, and a reference to store the index of the acquired
		 * image. Implement this function to ensure that the rendering system
		 * can acquire images from the swapchain for rendering operations.
		 *
		 * @param index The index of the image to acquire from the swapchain.
		 * @param device The Vulkan device used to perform the acquisition
		 * operation.
		 * @param imageAvailableSemaphore A Vulkan semaphore that will be
		 * signaled when the image is available for rendering.
		 * @param inFlightFence A Vulkan fence that will be signaled when the
		 * image is in flight (i.e., being rendered).
		 * @param imageIndex A reference to store the index of the acquired
		 * image from the swapchain.
		 *
		 * @return A Vulkan result indicating the success or failure of the
		 * image acquisition operation.
		 *
		 * @note This function is pure virtual and must be implemented by
		 * derived classes to handle the specific logic for acquiring images
		 * from the swapchain in the context of the rendering system.
		 */
		virtual VkResult aquireImage(uint32_t index, VkDevice device,
									 VkSemaphore imageAvailableSemaphore,
									 VkFence inFlightFence,
									 uint32_t &imageIndex) = 0;

		/**
		 * @brief Waits for the specified image in the swapchain to become
		 * available for rendering.
		 *
		 * This pure virtual function must be implemented by derived classes to
		 * handle waiting for a specific image in the swapchain to become
		 * available for rendering. The function takes the index of the image to
		 * wait for as a parameter. Implement this function to ensure that the
		 * rendering system can synchronize rendering operations with the
		 * availability of images in the swapchain.
		 *
		 * @param index The index of the image in the swapchain to wait for.
		 *
		 * @note This function is pure virtual and must be implemented by
		 * derived classes to handle the specific synchronization logic for
		 * waiting on images in the swapchain in the context of the rendering
		 * system.
		 */
		virtual void waitForImage(uint32_t index) = 0;

		/**
		 * A list of shared pointers to ASwapchainImage instances, which
		 * represent the images in the swapchain. Each ASwapchainImage
		 * encapsulates the properties and resources associated with a single
		 * image in the swapchain, such as the image itself, its view, and any
		 * associated memory. This vector is used to manage and access the
		 * images in the swapchain for rendering operations.
		 *
		 * @note The ASwapchainImage instances in this vector should be properly
		 * initialized and managed to ensure that the rendering system can
		 * effectively utilize the swapchain images for presenting rendered
		 * frames to the display.
		 */
		std::vector<std::shared_ptr<ASwapchainImage>> _swapchainImages;

		/**
		 * @brief Retrieves the view matrix for the specified image index in the
		 * swapchain.
		 *
		 * This pure virtual function must be implemented by derived classes to
		 * return the view matrix associated with the specified image index in
		 * the swapchain. The view matrix is used in rendering operations to
		 * transform world coordinates into view space for the corresponding
		 * image.
		 *
		 * @param index The index of the image in the swapchain for which to
		 * retrieve the view matrix.
		 *
		 * @return The view matrix as a utility::math::Matrix4x4F for the
		 * specified image index in the swapchain.
		 */
		virtual utility::math::Matrix4x4F getView(int index) const = 0;

		/**
		 * @brief Retrieves the projection matrix for the specified image index
		 * in the swapchain.
		 *
		 * This pure virtual function must be implemented by derived classes to
		 * return the projection matrix associated with the specified image
		 * index in the swapchain. The projection matrix is used in rendering
		 * operations to transform view space coordinates into clip space for
		 * the corresponding image.
		 *
		 * @param index The index of the image in the swapchain for which to
		 * retrieve the projection matrix.
		 *
		 * @return The projection matrix as a utility::math::Matrix4x4F for the
		 * specified image index in the swapchain.
		 */
		virtual utility::math::Matrix4x4F getProjection(int index) const = 0;

		protected:
		/**
		 * The Vulkan render pass associated with the swapchain context.
		 * The render pass defines the structure of the rendering operations,
		 * including the attachments, subpasses, and dependencies. It is
		 * essential for configuring the graphics pipeline and ensuring that
		 * rendering operations are performed correctly. This member variable
		 * should be properly initialized and managed to provide access to the
		 * render pass used in the swapchain context.
		 *
		 * @note The render pass should be created and configured according to
		 * the specific requirements of the rendering system and the swapchain
		 * configuration to ensure optimal performance and compatibility with
		 * the rendering operations being performed.
		 */
		VkRenderPass _renderPass;

		/**
		 * @brief Creates the Vulkan render pass for the swapchain context.
		 *
		 * This function is responsible for creating the Vulkan render pass that
		 * will be used in the swapchain context. The render pass defines the
		 * structure of the rendering operations, including the attachments,
		 * subpasses, and dependencies. The function takes a shared pointer to
		 * an ADeviceBackend instance, which provides access to the Vulkan
		 * device and other resources needed for render pass creation, as well
		 * as the number of samples for multisampling (MSAA). Implement this
		 * function to ensure that the render pass is properly configured and
		 * created according to the requirements of the rendering system and the
		 * swapchain configuration.
		 *
		 * @param deviceBackend A shared pointer to an ADeviceBackend instance,
		 * which provides access to the Vulkan device and other resources needed
		 * for render pass creation.
		 * @param msaaSamples The number of samples for multisampling (MSAA) to
		 * be used in the render pass. This parameter is essential for
		 * configuring the render pass to support multisampling if required by
		 * the rendering system.
		 *
		 * @note Implement this function to ensure that the render pass is
		 * properly created and configured according to the specific
		 * requirements of the rendering system and the swapchain configuration,
		 * including support for multisampling if necessary.
		 *
		 * @see VkRenderPass, ADeviceBackend
		 */
		void createRenderPass(
			const std::shared_ptr<ADeviceBackend> &deviceBackend,
			VkSampleCountFlagBits msaaSamples);

		/**
		 * @brief Selects the appropriate swapchain format from the available
		 * formats.
		 *
		 * This function is responsible for selecting the most suitable
		 * swapchain format from the list of available formats provided by the
		 * Vulkan physical device. The function takes a vector of int64_t values
		 * representing the available swapchain formats and returns the selected
		 * VkFormat that best matches the requirements of the rendering system.
		 * Implement this function to ensure that the selected swapchain format
		 * is compatible with the rendering operations and provides optimal
		 * performance for presenting rendered frames to the display.
		 * @param swapchainFormats A vector of int64_t values representing the
		 * available swapchain formats provided by the Vulkan physical device.
		 * These formats should be evaluated to determine the most suitable
		 * format for the swapchain.
		 *
		 * @return The selected VkFormat that best matches the requirements of
		 * the rendering system for the swapchain. This format should be
		 * compatible with the rendering operations and provide optimal
		 * performance for presenting rendered frames to the display.
		 *
		 * @note Implement this function to ensure that the selected swapchain
		 * format is chosen based on the specific requirements of the rendering
		 * system and the capabilities of the Vulkan physical device, taking
		 * into consideration factors such as color space, pixel format, and
		 * performance characteristics.
		 *
		 * @see VkFormat, Vulkan physical device swapchain formats
		 */
		VkFormat
			selectSwapchainFormat(const std::vector<int64_t> &swapchainFormats);

		/**
		 * @brief Finds a supported format from a list of candidates based on
		 * the specified tiling and feature requirements.
		 *
		 * This function is responsible for finding a supported format from a
		 * list of candidate formats based on the specified tiling and feature
		 * requirements. The function takes a Vulkan physical device, a vector
		 * of candidate VkFormat values, the desired VkImageTiling, and the
		 * required VkFormatFeatureFlags as parameters. It queries the Vulkan
		 * physical device to determine which of the candidate formats is
		 * supported with the specified tiling and features, and returns the
		 * first suitable format found. Implement this function to ensure that a
		 * compatible format is selected for use in the swapchain context or
		 * other rendering operations that require specific format support.
		 *
		 * @param physicalDevice The Vulkan physical device used to query the
		 * available formats and their support for the specified tiling and
		 * feature requirements. This device should be properly initialized and
		 * should support the necessary features to ensure that a suitable
		 * format can be found.
		 * @param candidates A vector of candidate VkFormat values that should
		 * be evaluated to determine which format is supported with the
		 * specified tiling and features. These candidates should be chosen
		 * based on the requirements of the rendering system and the
		 * capabilities of the Vulkan physical device.
		 * @param tiling The desired VkImageTiling that the selected format
		 * should support. This parameter specifies the tiling arrangement for
		 * images using the selected format and should be chosen based on the
		 * intended usage of the images in the rendering system.
		 * @param features The required VkFormatFeatureFlags that the selected
		 * format should support. This parameter specifies the features that the
		 * format must support, such as sampling, filtering, or rendering
		 * capabilities, and should be chosen based on the specific requirements
		 * of the rendering operations that will utilize the selected format.
		 *
		 * @return The selected VkFormat that is supported with the specified
		 * tiling and features from the list of candidates. This format should
		 * be compatible with the rendering operations and provide optimal
		 * performance for the intended usage in the swapchain context or other
		 * rendering operations.
		 */
		VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice,
									 const std::vector<VkFormat> &candidates,
									 VkImageTiling tiling,
									 VkFormatFeatureFlags features);

		private:
	};
}	 // namespace evan