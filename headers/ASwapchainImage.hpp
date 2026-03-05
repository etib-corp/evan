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
         * @brief Creates image views for the swapchain images.
         *
         * This function iterates through the swapchain images and creates image views
         * for each image. Image views are used to describe how the images should be
         * accessed and are necessary for rendering operations. The created image views
         * are stored in the _imageViews vector for later use in the rendering pipeline.
         */
		void createImageViews(VkDevice logicalDevice);

        /**
         * @brief Finds the queue family indices for a given Vulkan physical device.
         *
         * This function queries the Vulkan API to find the indices of the queue families
         * that support graphics and presentation operations for the specified physical device.
         * It returns a QueueFamilyIndices structure containing the indices of the graphics
         * and presentation queue families. This information is essential for setting up the
         * rendering pipeline and ensuring that the application can properly present rendered
         * images to the screen.
         */
        virtual evan::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) = 0;

		protected:
		std::vector<VkFence>
			_inFlightFences;	/// Fences to synchronize rendering operations
								/// for each swapchain image.
		VkImage
			_depthImage;	/// The depth image, which is used for depth testing
							/// during rendering. It stores depth information
							/// for each pixel and is typically used in 3D
							/// rendering to determine the visibility of objects
							/// based on their distance from the camera.
		VkImageView
			_depthView;	   /// The image view for the depth image, which
						   /// provides a way to access the depth image's data
						   /// in a specific format and layout. It is used to
						   /// bind the depth image to the graphics pipeline and
						   /// allows shaders to read from or write to the depth
						   /// image during rendering operations.
		VkDeviceMemory
			_depthMemory;	 /// The memory allocated for the depth image, which
							 /// is used to store the depth information on the
							 /// GPU. It is allocated and managed by the Vulkan
							 /// API and is associated with the depth image to
							 /// enable efficient access during rendering
							 /// operations.
		VkImage _colorImage;	/// The color image, which is used to store the
								/// rendered color output that will be presented
								/// to the screen. It is typically created as
								/// part of the swapchain and is used as the
								/// target for rendering operations.
		VkImageView
			_colorView;	   /// The image view for the color image, which
						   /// provides a way to access the color image's data
						   /// in a specific format and layout. It is used to
						   /// bind the color image to the graphics pipeline and
						   /// allows shaders to read from or write to the color
						   /// image during rendering operations.
		VkDeviceMemory
			_colorMemory;	 /// The memory allocated for the color image, which
							 /// is used to store the rendered color output on
							 /// the GPU. It is allocated and managed by the
							 /// Vulkan API and is associated with the color
							 /// image to enable efficient access during
							 /// rendering operations.
		std::vector<VkImageView>
			_imageViews;	/// A vector of image views for the swapchain
							/// images, which provides a way to access the
							/// swapchain images' data in specific formats and
							/// layouts. These image views are used to bind the
							/// swapchain images to the graphics pipeline and
							/// allow shaders to read from or write to the
							/// swapchain images during rendering operations.
		VkExtent2D _extent;	   /// The extent of the swapchain images, which
							   /// defines the width and height of the images in
							   /// pixels. It is typically determined based on
							   /// the surface capabilities and the desired
							   /// resolution for rendering.
		VkFormat
			_format;	/// The format of the swapchain images, which specifies
						/// the color format and layout of the pixel data. It is
						/// determined based on the surface capabilities and the
						/// desired format for rendering output.
		std::vector<VkImage>
			_images;	/// A vector of Vulkan images that represent the
						/// swapchain images. These images are created as part
						/// of the swapchain and are used as the targets for
						/// rendering operations. Each
		std::vector<VkFramebuffer>
			_framebuffers;	  /// A vector of Vulkan framebuffers that are
							  /// associated with the swapchain images.
							  /// Framebuffers are used to define the render
							  /// targets for rendering operations and are
							  /// typically created by combining the color and
							  /// depth image views. Each framebuffer
							  /// corresponds to a specific swapchain image and
							  /// is used during the rendering process to
							  /// specify where the rendered output should be
							  /// stored.
		private:
		/**
		 * @brief Creates a Vulkan image view for a given image.
		 *
		 * This function sets up and creates a Vulkan image view, which is used
		 * to describe how an image resource should be accessed. It specifies
		 * the format, view type, and subresource range for the image view.
		 *
		 * @param image The Vulkan image for which the image view is created.
		 * @param format The format of the image view (e.g.,
		 * VK_FORMAT_R8G8B8A8_SRGB).
		 * @param aspectFlags Specifies which aspect(s) of the image are
		 * included in the view (e.g., VK_IMAGE_ASPECT_COLOR_BIT for color
		 * images).
		 * @param logicalDevice The Vulkan logical device used to create the
		 * image view.
		 *
		 * @return A VkImageView handle representing the created image view.
		 *
		 * @throws std::runtime_error If the image view creation fails.
		 */

		VkImageView createImageView(VkImage image, VkFormat format,
									VkImageAspectFlags aspectFlags,
									VkDevice logicalDevice, uint32_t mipLevels);
	};
}	 // namespace evan
