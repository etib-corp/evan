/*
** EPITECH PROJECT, 2026
** evan
** File description:
** ADeviceBackend
*/

#pragma once

#include "EvanPlatform.hpp"
#include "Version.hpp"

#include "IPlatform.hpp"
#include "QueueFamilyIndices.hpp"

#include <set>
#include <iostream>

namespace evan
{
	class ADeviceBackend
	{
		public:
		/**
		 * @struct CopyBufferProperties
		 * @brief Encapsulates properties required for copying data between
		 * Vulkan buffers.
		 *
		 * This structure holds all necessary Vulkan objects and parameters
		 * needed to perform a buffer-to-buffer copy operation, such as the
		 * logical device, command pool, graphics queue, source and destination
		 * buffers, and the size of the data to copy.
		 *
		 */
		struct CopyBufferProperties {
			/*
			 * @brief The Vulkan logical device used for command buffer
			 * operations.
			 */
			VkDevice _logicalDevice;
			/*
			 * @brief The Vulkan command pool used to allocate command buffers
			 * for the copy operation.
			 */
			VkCommandPool _commandPool;
			/*
			 * @brief The Vulkan graphics queue used to submit the command
			 * buffer for execution.
			 */
			VkQueue _graphicsQueue;
			/*
			 * @brief The Vulkan buffer from which data will be copied.
			 */
			VkBuffer _srcBuffer;
			/*
			 * @brief The Vulkan buffer to which data will be copied.
			 */
			VkBuffer _dstBuffer;
			/*
			 * @brief The size of the data to copy in bytes.
			 */
			VkDeviceSize _size;
		};

		/**
		 * @brief Properties required to copy data from a buffer to an image in
		 * Vulkan.
		 *
		 * This structure encapsulates all necessary Vulkan objects and
		 * parameters needed to perform a buffer-to-image copy operation.
		 *
		 */
		struct CopyBufferToImageProperties {
			/*
			 * @brief The Vulkan logical device used for command buffer
			 * operations.
			 */
			VkDevice _logicalDevice;
			/*
			 * @brief The Vulkan command pool used to allocate command buffers
			 * for the copy operation.
			 */
			VkCommandPool _commandPool;
			/*
			 * @brief The Vulkan graphics queue used to submit the command
			 * buffer for execution.
			 */
			VkQueue _graphicsQueue;
			/*
			 * @brief The Vulkan buffer containing the image data to be copied.
			 */
			VkBuffer _buffer;
			/*
			 * @brief The Vulkan image to which the data will be copied.
			 */
			VkImage _image;
			/*
			 * @brief The layout of the image before the copy operation.
			 */
			uint32_t _width;
			/*
			 * @brief The layout of the image before the copy operation.
			 */
			uint32_t _height;
		};

		/**
		 * @struct TransitionImageLayoutProperties
		 * @brief Encapsulates the properties required to perform an image
		 * layout transition in Vulkan.
		 *
		 * This structure holds all necessary Vulkan objects and parameters
		 * needed to transition the layout of a VkImage, such as device, command
		 * pool, queue, image, format, old and new layouts, and the number of
		 * mipmap levels.
		 *
		 */
		struct TransitionImageLayoutProperties {
			/*
			 * @brief The Vulkan logical device used for command buffer
			 * operations.
			 */
			VkDevice _logicalDevice;
			/*
			 * @brief The Vulkan command pool used to allocate command buffers
			 * for the layout transition.
			 */
			VkCommandPool _commandPool;
			/*
			 * @brief The Vulkan graphics queue used to submit the command
			 * buffer for execution.
			 */
			VkQueue _graphicsQueue;
			/*
			 * @brief The Vulkan image whose layout is to be transitioned.
			 */
			VkImage _image;
			/*
			 * @brief The format of the image being transitioned.
			 */
			VkFormat _format;
			/*
			 * @brief The old layout of the image before the transition.
			 */
			VkImageLayout _oldLayout;
			/*
			 * @brief The new layout of the image after the transition.
			 */
			VkImageLayout _newLayout;
			/*
			 * @brief The number of mipmap levels in the image.
			 */
			uint32_t _mipLevels;
		};

		/**
		 * @struct CreateImageProperties
		 * @brief Encapsulates the properties required to create a Vulkan image
		 * and allocate its memory.
		 *
		 * This structure holds all necessary parameters for creating a VkImage
		 * object and allocating its associated device memory in Vulkan. It
		 * includes device handles, image dimensions, format, usage flags, and
		 * references to the resulting image and memory objects.
		 *
		 */
		struct CreateImageProperties {
			/*
			 * @brief The Vulkan logical device used for image creation and
			 * memory allocation.
			 */
			VkDevice _logicalDevice;
			/*
			 * @brief The Vulkan physical device used to determine memory
			 * properties.
			 */
			VkPhysicalDevice _physicalDevice;
			/*
			 * @brief The width of the image in pixels.
			 */
			uint32_t _width;
			/*
			 * @brief The height of the image in pixels.
			 */
			uint32_t _height;
			/*
			 * @brief The number of mipmap levels for the image.
			 */
			uint32_t _mipLevels;
			/*
			 * @brief The number of samples for multisample anti-aliasing
			 * (MSAA).
			 */
			VkSampleCountFlagBits _numSamples;
			/*
			 * @brief The format of the image (e.g., VK_FORMAT_R8G8B8A8_SRGB).
			 */
			VkFormat _format;
			/*
			 * @brief The tiling arrangement of the image (e.g.,
			 * VK_IMAGE_TILING_OPTIMAL).
			 */
			VkImageTiling _tiling;
			/*
			 * @brief The usage flags for the image (e.g.,
			 * VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT).
			 */
			VkImageUsageFlags _usage;
			/*
			 * @brief The memory property flags for the image (e.g.,
			 * VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT).
			 */
			VkMemoryPropertyFlags _properties;
			/*
			 * @brief A reference to the VkImage object that will be created.
			 */
			VkImage &_image;
			/*
			 * @brief A reference to the VkDeviceMemory object that will be
			 * allocated for the image.
			 */
			VkDeviceMemory &_imageMemory;
		};

		/**
		 * @brief Properties required for creating a Vulkan buffer and
		 * allocating its memory.
		 *
		 * This structure encapsulates all necessary parameters for buffer
		 * creation, including device handles, buffer size, usage flags, memory
		 * properties, and references to the resulting buffer and its memory.
		 *
		 */
		struct CreateBufferProperties {
			/*
			 * @brief The Vulkan logical device used for buffer creation and
			 * memory allocation.
			 */
			VkDevice _logicalDevice;
			/*
			 * @brief The Vulkan physical device used to determine memory
			 * properties.
			 */
			VkPhysicalDevice _physicalDevice;
			/*
			 * @brief The size of the buffer in bytes.
			 */
			VkDeviceSize _size;
			/*
			 * @brief The usage flags for the buffer (e.g.,
			 * VK_BUFFER_USAGE_VERTEX_BUFFER_BIT).
			 */
			VkBufferUsageFlags _usage;
			/*
			 * @brief The memory property flags for the buffer (e.g.,
			 * VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT).
			 */
			VkMemoryPropertyFlags _properties;
			/*
			 * @brief A reference to the VkBuffer object that will be created.
			 */
			VkBuffer &_buffer;
			/*
			 * @brief A reference to the VkDeviceMemory object that will be
			 * allocated for the buffer.
			 */
			VkDeviceMemory &_bufferMemory;
		};

		virtual ~ADeviceBackend();

		virtual void createInstance(const IPlatform &platform,
									const std::string &appName,
									Version &appVersion) = 0;

		virtual uint32_t countSwapchainFormats() const = 0;
		virtual std::vector<int64_t>
			enumerateSwapchainFormats(uint32_t swapchainFormatCount) const = 0;

		virtual void createLogicalDevice() = 0;

		virtual void pickPhysicalDevice() = 0;

		/**
		 * @brief Finds the queue family indices for the Vulkan physical device.
		 *
		 * This function queries the Vulkan API to find the indices of the queue
		 * families that support graphics and presentation operations. It
		 * returns a QueueFamilyIndices structure containing the indices of the
		 * graphics and presentation queue families, if they are found. If the
		 * required queue families are not found, the corresponding optional
		 * values in the QueueFamilyIndices structure will be empty.
		 */
		virtual evan::QueueFamilyIndices findQueueFamilies() = 0;

		/**
		 * @brief Retrieves the available Vulkan layers on the system.
		 *
		 * This function queries the Vulkan API to enumerate the available
		 * instance layers and returns a vector of VkLayerProperties structures
		 * containing information about each layer.
		 */
		std::vector<VkLayerProperties> getAvailableLayers();

		void createBuffer(const CreateBufferProperties &properties) const;

		void transitionImageLayout(
			const TransitionImageLayoutProperties &properties) const;
		VkCommandBuffer
			beginSingleTimeCommands(VkDevice logicalDevice,
									VkCommandPool commandPool) const;
		bool hasStencilComponent(VkFormat format) const;
		void endSingleTimeCommands(VkDevice logicalDevice,
								   VkCommandPool commandPool,
								   VkQueue graphicsQueue,
								   VkCommandBuffer commandBuffer) const;

		uint32_t findMemoryType(VkPhysicalDevice physicalDevice,
								uint32_t typeFilter,
								VkMemoryPropertyFlags properties) const;

		void createImage(const CreateImageProperties &properties) const;

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
									VkImageAspectFlags aspectFlags, uint32_t mipLevels) const;


		void copyBufferToImage(
			const CopyBufferToImageProperties &properties) const;

		void copyBuffer(const CopyBufferProperties &properties) const;

		VkInstance
			_VkInstance = VK_NULL_HANDLE;	/// The Vulkan instance, which is the connection
							/// between the application and the Vulkan library.
							/// It is used to create and manage Vulkan resources
							/// and to query the capabilities of the Vulkan
							/// implementation.
		VkPhysicalDevice
			_physicalDevice = VK_NULL_HANDLE;	/// The physical device, which represents a
								/// Vulkan-compatible GPU. It is used to query
								/// the capabilities of the GPU and to create
								/// logical devices for rendering operations.
		VkDevice _device = VK_NULL_HANDLE;		/// The logical device, which represents an
								/// instance of a physical device and is used to
								/// perform rendering operations.
		private:
	};
}	 // namespace evan