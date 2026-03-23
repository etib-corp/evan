/*
** ETIB PROJECT, 2026
** evan
** File description:
** ASwapchainImage
*/

#pragma once

#include "EvanPlatform.hpp"

#include "QueueFamilyIndices.hpp"
#include "Version.hpp"

namespace evan
{
	class DeviceContext;
	class ADeviceBackend;

	/**
	 * @brief ASwapchainImage is a class that represents an image in the
	 * swapchain of the Evan engine. It serves as a base class for specific
	 * implementations of swapchain images that are compatible with different
	 * graphics APIs (e.g., DirectX, Vulkan, OpenGL). The ASwapchainImage class
	 * provides an interface for managing the properties and behavior of
	 * swapchain images, which are used to present rendered content to the
	 * screen.
	 *
	 * @note The ASwapchainImage class is designed to be inherited by specific
	 * implementations for different graphics APIs. It defines the necessary
	 * functions and properties that must be implemented by derived classes to
	 * ensure proper functionality of swapchain images within the Evan engine's
	 * rendering system.
	 */
	class ASwapchainImage
	{
		public:
		virtual ~ASwapchainImage();

		/**
		 * @brief Destroys the swapchain image and releases associated
		 * resources.
		 *
		 * This pure virtual function must be implemented by derived classes to
		 * handle the destruction of the swapchain image and the release of any
		 * resources associated with it. The function takes a Vulkan device as a
		 * parameter, which is used to perform the necessary cleanup operations.
		 * Implement this function to ensure that all resources related to the
		 * swapchain image are properly released when the image is no longer
		 * needed.
		 *
		 * @param device The Vulkan device used to perform the cleanup
		 * operations for the swapchain image.
		 *
		 * @note This function is pure virtual and must be implemented by
		 * derived classes to handle the specific logic for destroying swapchain
		 * images in the context of the rendering system.
		 */
		virtual void destroy(VkDevice device) = 0;

		/**
		 * @brief Fills the VkPresentInfoKHR structure with the necessary
		 * information for presenting the swapchain image.
		 *
		 * This pure virtual function must be implemented by derived classes to
		 * populate the VkPresentInfoKHR structure with the appropriate
		 * information required for presenting the swapchain image to the
		 * screen. The function takes a reference to a VkPresentInfoKHR
		 * structure as a parameter, which should be filled with the necessary
		 * details such as the swapchain, image index, and synchronization
		 * primitives. Implement this function to ensure that the rendering
		 * system can correctly present rendered frames using the swapchain
		 * images.
		 *
		 * @param presentInfo A reference to a VkPresentInfoKHR structure that
		 * should be filled with the necessary information for presenting the
		 * swapchain image.
		 *
		 * @note This function is pure virtual and must be implemented by
		 * derived classes to handle the specific logic for filling the
		 * presentation information for swapchain images in the context of the
		 * rendering system.
		 */
		virtual void fillPresentInfo(VkPresentInfoKHR &presentInfo) const = 0;

		/**
		 * @brief Retrieves the Vulkan framebuffer associated with the swapchain
		 * image at the specified index.
		 *
		 * This function returns the Vulkan framebuffer that is associated with
		 * the swapchain image at the given index. The framebuffer is used as a
		 * render target during rendering operations and is typically created by
		 * combining the color and depth image views. Implement this function to
		 * provide access to the framebuffer for the specified swapchain image,
		 * which is essential for rendering operations that target that image.
		 *
		 * @param index The index of the swapchain image for which to retrieve
		 * the framebuffer.
		 *
		 * @return The Vulkan framebuffer associated with the swapchain image at
		 * the specified index.
		 */
		VkFramebuffer getFramebuffer(uint32_t index) const;

		/**
		 * @brief Retrieves the extent (width and height) of the swapchain
		 * images.
		 *
		 * This function returns the extent of the swapchain images, which
		 * defines the width and height of the images in pixels. The extent is
		 * typically determined based on the surface capabilities and the
		 * desired resolution for rendering. Implement this function to provide
		 * access to the extent of the swapchain images, which is essential for
		 * configuring the rendering pipeline and ensuring that rendered content
		 * is correctly sized for presentation.
		 *
		 * @return The extent of the swapchain images, represented as a
		 * VkExtent2D structure containing the width and height in pixels.
		 */
		VkExtent2D getExtent() const;

		/**
		 * @brief Retrieves the number of framebuffers associated with the
		 * swapchain images.
		 *
		 * This function returns the number of framebuffers that are associated
		 * with the swapchain images. Each framebuffer corresponds to a specific
		 * swapchain image and is used during the rendering process to specify
		 * where the rendered output should be stored. Implement this function
		 * to provide access to the count of framebuffers, which is essential
		 * for managing rendering operations and ensuring that the correct
		 * number of framebuffers is available for rendering to the swapchain
		 * images.
		 *
		 * @return The number of framebuffers associated with the swapchain
		 * images.
		 */
		uint32_t getFramebufferCount() const;

		protected:
		/**
		 * @brief Creates image views for the swapchain images.
		 *
		 * @param deviceBackend A reference to the ADeviceBackend instance,
		 * which is used to create the image views. The ADeviceBackend provides
		 * the necessary Vulkan device and other resources required for image
		 * view creation.
		 *
		 * This function iterates through the swapchain images and creates image
		 * views for each image. Image views are used to describe how the images
		 * should be accessed and are necessary for rendering operations. The
		 * created image views are stored in the _imageViews vector for later
		 * use in the rendering pipeline.
		 */
		void createImageViews(const ADeviceBackend &deviceBackend);

		/**
		 * @brief Creates color resources for the swapchain images.
		 *
		 * This function creates the necessary color resources for the swapchain
		 * images, including the color image, its associated image view, and the
		 * memory allocation for the color image. These resources are essential
		 * for rendering operations, as they serve as the targets for rendering
		 * output. The function takes the logical device, physical device, and
		 * the number of samples for multisampling.
		 */
		void createColorResources(const ADeviceBackend &deviceBackend,
								  VkSampleCountFlagBits msaaSamples);

		/**
		 * @brief Creates depth resources for the swapchain images.
		 *
		 * This function creates the necessary depth resources for the swapchain
		 * images, including the depth image, its associated image view, and the
		 * memory allocation for the depth image
		 *
		 * These resources are essential for depth testing during rendering
		 * operations, as they store depth information for each pixel. The
		 * function takes a DeviceContext as a parameter, which provides access
		 * to the Vulkan device and other resources needed for creating the
		 * depth resources.
		 */
		void createDepthResources(const DeviceContext &deviceContext);

		/**
		 * @brief Creates framebuffers for the swapchain images.
		 *
		 * This function creates the Vulkan framebuffers that are associated
		 * with the swapchain images. Framebuffers are used to define the render
		 * targets for rendering operations and are typically created by
		 * combining the color and depth image views. Each framebuffer
		 * corresponds to a specific swapchain image and is used during the
		 * rendering process to specify where the rendered output should be
		 * stored. The function takes the logical device and render pass as
		 * parameters, which are necessary for framebuffer creation.
		 *
		 * @param logicalDevice The Vulkan logical device used to create the
		 * framebuffers. This device should be properly initialized and should
		 * support the necessary features for framebuffer creation.
		 * @param renderPass The Vulkan render pass that defines the structure
		 * of the rendering operations. This render pass should be compatible
		 * with the attachments used in the framebuffers.
		 */
		void createFramebuffers(VkDevice logicalDevice,
								VkRenderPass renderPass);

		/**
		 * @brief Creates the Vulkan images for the swapchain.
		 *
		 * This function is responsible for creating the Vulkan images that
		 * represent the swapchain images. These images are created as part of
		 * the swapchain and are used as the targets for rendering operations.
		 * The function takes the logical device and the swapchain as
		 * parameters, which are necessary for image creation. Implement this
		 * function to ensure that the Vulkan images for the swapchain are
		 * properly created and configured according to the requirements of the
		 * rendering system.
		 *
		 * @param logicalDevice The Vulkan logical device used to create the
		 * images for the swapchain. This device should be properly initialized
		 * and should support the necessary features for image creation.
		 * @param swapchain The Vulkan swapchain for which the images are being
		 * created. This swapchain should be properly initialized and should
		 * provide the necessary information for creating the images, such as
		 * the number of images, their format, and their extent.
		 */
		void createImages(VkDevice logicalDevice, VkSwapchainKHR swapchain);

		/**
		 * The depth image, which is used for depth testing
		 * during rendering. It stores depth information
		 * for each pixel and is typically used in 3D
		 * rendering to determine the visibility of objects
		 * based on their distance from the camera.
		 */
		VkImage _depthImage;

		/**
		 * The image view for the depth image, which provides
		 * a way to access the depth image's data in a specific
		 * format and layout. It is used to bind the depth image to
		 * the graphics pipeline and allows shaders to read from or
		 * write to the depth image during rendering operations.
		 */
		VkImageView _depthView;

		/**
		 * The memory allocated for the depth image, which is used to
		 * store the depth information on the GPU. It is allocated and
		 * managed by the Vulkan API and is associated with the depth
		 * image to enable efficient access during rendering operations.
		 */
		VkDeviceMemory _depthMemory;

		/**
		 * The color image, which is used to store the
		 * rendered color output that will be presented
		 * to the screen. It is typically created as
		 * part of the swapchain and is used as the
		 * target for rendering operations.
		 */
		VkImage _colorImage;

		/**
		 * The image view for the color image, which
		 * provides a way to access the color image's
		 * data in a specific format and layout. It is used
		 * to bind the color image to the graphics pipeline and
		 * allows shaders to read from or write to the color
		 * image during rendering operations.
		 */
		VkImageView _colorView;

		/**
		 * The memory allocated for the color image, which is used to
		 * store the rendered color output on the GPU. It is allocated and
		 * managed by the Vulkan API and is associated with the color image
		 * to enable efficient access during rendering operations.
		 */
		VkDeviceMemory _colorMemory;

		/**
		 * A vector of Vulkan images that represent the swapchain images.
		 * These images are created as part of the swapchain and are used as
		 * the targets for rendering operations. Each image in this vector
		 * corresponds to a specific swapchain image and is used during the
		 * rendering process to specify where the rendered output should be
		 * stored for presentation to the screen.
		 */
		std::vector<VkImageView> _imageViews;

		/**
		 * The extent of the swapchain images, which
		 * defines the width and height of the images in
		 * pixels. It is typically determined based on
		 * the surface capabilities and the desired
		 * resolution for rendering.
		 */
		VkExtent2D _extent;

		/**
		 * The format of the swapchain images, which specifies
		 * the color format and layout of the pixel data. It is
		 * determined based on the surface capabilities and the
		 * desired format for rendering output.
		 */
		VkFormat _format;

		/**
		 * A vector of Vulkan images that represent the
		 * swapchain images. These images are created as part
		 * of the swapchain and are used as the targets for
		 * rendering operations. Each image in this vector corresponds
		 * to a specific swapchain image and is used during the rendering
		 * process to specify where the rendered output should be stored
		 * for presentation to the screen.
		 */
		std::vector<VkImage> _images;

		/**
		 * A vector of Vulkan framebuffers that are associated with the
		 * swapchain images. Framebuffers are used to define the render
		 * targets for rendering operations and are typically created by
		 * combining the color and depth image views. Each framebuffer
		 * corresponds to a specific swapchain image and is used during
		 * the rendering process to specify where the rendered output
		 * should be stored.
		 */
		std::vector<VkFramebuffer> _framebuffers;

		private:
		/**
		 * @brief Finds a supported depth format for the swapchain context.
		 *
		 * This function is responsible for finding a supported depth format
		 * that can be used in the swapchain context for depth buffering. The
		 * function takes a Vulkan physical device as a parameter and queries
		 * the available formats to determine which one is suitable for depth
		 * buffering based on the requirements of the rendering system.
		 * Implement this function to ensure that a compatible depth format is
		 * selected for use in the swapchain context, which is essential for
		 * proper depth testing and rendering of 3D scenes.
		 *
		 * @param physicalDevice The Vulkan physical device used to query the
		 * available formats for depth buffering. This device should be properly
		 * initialized and should support the necessary features for depth
		 * buffering to ensure that a suitable depth format can be found.
		 *
		 * @return The selected VkFormat that is supported for depth buffering
		 * in the swapchain context. This format should be compatible with the
		 * rendering operations and provide optimal performance for depth
		 * testing in 3D scenes.
		 */
		VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

		/**
		 * @brief Finds a supported format from a list of candidates based on
		 * the specified tiling and features.
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
	};
}	 // namespace evan
