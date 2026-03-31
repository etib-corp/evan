/*
** ETIB PROJECT, 2026
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
	class ASwapchainContext;
	class ASwapchainImage;

	/**
	 * @class ADeviceBackend
	 *
	 * @brief Abstract base class for Vulkan device backends.
	 *
	 * This class defines the interface for Vulkan device backends, providing
	 * common functionality for managing Vulkan resources such as buffers,
	 * images, and command buffers. It also includes structures for
	 * encapsulating properties required for various Vulkan operations, such as
	 * copying buffers and transitioning image layouts. Concrete implementations
	 * of this class will provide platform-specific logic for creating Vulkan
	 * instances, devices, and handling frame processing.
	 *
	 * @note This class is designed to be extended by platform-specific Vulkan
	 * device backends, which will implement the pure virtual functions defined
	 * here.
	 */
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
		 * @note TODO: Remove useless properties (such as the logical device
		 * which is already a member of the class).
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
		 * @note TODO: Remove useless properties (such as the logical device
		 * which is already a member of the class).
		 */
		struct CopyBufferToImageProperties {
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
		 * @note TODO: Remove useless properties (such as the logical device
		 * which is already a member of the class).
		 */
		struct TransitionImageLayoutProperties {
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
		 * @note TODO: Remove useless properties (such as the logical device
		 * which is already a member of the class).
		 */
		struct CreateImageProperties {
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
		 * @note TODO: Remove useless properties (such as the logical device
		 * which is already a member of the class).
		 */
		struct CreateBufferProperties {
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

		/**
		 * @brief Virtual destructor for the ADeviceBackend class.
		 */
		virtual ~ADeviceBackend();

		/**
		 * @brief Counts the number of available swapchain formats for the
		 * Vulkan device.
		 *
		 * This function queries the Vulkan API to determine how many swapchain
		 * formats are supported by the physical device. The count of swapchain
		 * formats is essential for creating a swapchain that is compatible with
		 * the device's capabilities.
		 *
		 * @return uint32_t The number of available swapchain formats supported
		 * by the Vulkan device.
		 */
		virtual uint32_t countSwapchainFormats() const = 0;

		/**
		 * @brief Enumerates the available swapchain formats for the Vulkan
		 * device.
		 *
		 * This function retrieves the list of supported swapchain formats from
		 * the Vulkan API. It takes the count of swapchain formats as a
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
		virtual std::vector<int64_t>
			enumerateSwapchainFormats(uint32_t swapchainFormatCount) const = 0;

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
		 * @brief Preprocesses a frame before presentation.
		 *
		 * This function is called before the presentation of a frame and allows
		 * for any necessary preprocessing steps to be performed on the
		 * swapchain context. It takes a reference to an ASwapchainContext
		 * object, which contains information about the current state of the
		 * swapchain and the images being presented. The function returns a
		 * boolean value indicating whether the preprocessing was successful and
		 * if the frame is ready for presentation.
		 *
		 * @param swapchainContext A reference to the ASwapchainContext object
		 * containing information about the swapchain and images for the current
		 * frame.
		 *
		 * @return true if preprocessing was successful and the frame is ready
		 * for presentation, false otherwise
		 */
		virtual bool preprocessFrame(ASwapchainContext &swapchainContext) = 0;

		/**
		 * @brief Processes a frame for presentation.
		 *
		 * This function is called to process a frame for presentation and
		 * handles the actual presentation logic. It takes the presentation
		 * information and the swapchain image to be presented.
		 *
		 * @param presentInfo The presentation information for the frame.
		 * @param swapchainImage The swapchain image to be presented.
		 *
		 * @return true if the frame was processed successfully, false
		 * otherwise.
		 */
		virtual bool processFrame(VkPresentInfoKHR presentInfo,
								  ASwapchainImage &swapchainImage) = 0;

		/**
		 * @brief Postprocesses a frame after presentation.
		 *
		 * This function is called after the presentation of a frame and allows
		 * for any necessary postprocessing steps to be performed on the
		 * swapchain context. It takes a reference to an ASwapchainContext
		 * object, which contains information about the current state of the
		 * swapchain and the images being presented. The function returns a
		 * boolean value indicating whether the postprocessing was successful.
		 *
		 * @param swapchainContext A reference to the ASwapchainContext object
		 * containing information about the swapchain and images for the current
		 * frame.
		 *
		 * @return true if postprocessing was successful, false otherwise.
		 */
		virtual bool postprocessFrame(ASwapchainContext &swapchainContext) = 0;

		/**
		 * @brief Creates a Vulkan buffer and allocates memory for it.
		 *
		 * This function is responsible for creating a Vulkan buffer based on
		 * the provided properties, which include the logical device, physical
		 * device, buffer size, usage flags, memory properties, and references
		 * to the resulting buffer and its memory. The function will create the
		 * buffer and allocate the necessary memory for it, ensuring that it is
		 * ready for use in Vulkan operations.
		 *
		 * @param properties A structure containing all necessary parameters for
		 * buffer creation and memory allocation.
		 *
		 * @throws std::runtime_error If buffer creation or memory allocation
		 * fails, an exception will be thrown with details about the failure.
		 *
		 */
		void createBuffer(const CreateBufferProperties &properties) const;

		/**
		 * @brief Transitions the layout of a Vulkan image.
		 *
		 * This function is responsible for transitioning the layout of a Vulkan
		 * image from one state to another. It takes the necessary properties
		 * for the layout transition and performs the operation on the specified
		 * image.
		 *
		 * @param properties A structure containing all necessary parameters for
		 * the image layout transition.
		 *
		 * @throws std::runtime_error If the layout transition fails, an
		 * exception will be thrown with details about the failure.
		 */
		void transitionImageLayout(
			const TransitionImageLayoutProperties &properties) const;

		/**
		 * @brief Begins recording a single-use command buffer for one-time
		 * operations.
		 *
		 * This function allocates a command buffer from the specified command
		 * pool and begins recording it with the
		 * VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT flag. The returned
		 * command buffer can be used for short operations such as copying
		 * buffers or transitioning image layouts, and should be ended and
		 * submitted for execution after recording.
		 *
		 * @param commandPool The Vulkan command pool from which to allocate the
		 * command buffer.
		 *
		 * @return VkCommandBuffer A handle to the allocated and begun command
		 * buffer ready for recording
		 */
		VkCommandBuffer
			beginSingleTimeCommands(VkCommandPool commandPool) const;

		/**
		 * @brief Ends recording of a single-use command buffer and submits it
		 * for execution.
		 *
		 * This function ends the recording of the provided command buffer,
		 * submits it to the specified graphics queue, and waits for the queue
		 * to become idle before returning. It is typically used in conjunction
		 * with beginSingleTimeCommands() for short operations that require
		 * immediate execution, such as copying buffers or transitioning image
		 * layouts.
		 *
		 * @param commandPool The Vulkan command pool from which the command
		 * buffer was allocated.
		 * @param graphicsQueue The Vulkan graphics queue to which the command
		 * buffer will be submitted for execution.
		 * @param commandBuffer The Vulkan command buffer that has been recorded
		 * and is ready to be ended and submitted.
		 *
		 * @note TODO: Remove useless properties (such as the logical device
		 * which is already a member of the class).
		 */
		void endSingleTimeCommands(VkCommandPool commandPool,
								   VkQueue graphicsQueue,
								   VkCommandBuffer commandBuffer) const;

		/**
		 * @brief Creates a Vulkan image and allocates memory for it.
		 *
		 * This function is responsible for creating a Vulkan image based on the
		 * provided properties, which include the logical device, physical
		 * device, image dimensions, format, tiling, usage flags, memory
		 * properties, and references to the resulting image and its memory. The
		 * function will create the image and allocate the necessary memory for
		 * it, ensuring that it is ready for use in Vulkan operations.
		 *
		 * @param properties A structure containing all necessary parameters for
		 * image creation and memory allocation.
		 *
		 * @throws std::runtime_error If image creation or memory allocation
		 * fails, an exception will be thrown with details about the failure.
		 */
		void createImage(const CreateImageProperties &properties) const;

		/**
		 * @brief Copies data from a Vulkan buffer to a Vulkan image.
		 *
		 * This function is responsible for copying data from a Vulkan buffer to
		 * a Vulkan image based on the provided properties, which include the
		 * logical device, command pool, graphics queue, source buffer,
		 * destination image, and the dimensions of the image. The function will
		 * record and submit a command buffer that performs the copy operation,
		 * ensuring that the data is transferred correctly from the buffer to
		 * the image.
		 *
		 * @param properties A structure containing all necessary parameters for
		 * copying data from a buffer to an image.
		 *
		 * @throws std::runtime_error If the copy operation fails, an exception
		 * will be thrown with details about the failure.
		 */
		void copyBufferToImage(
			const CopyBufferToImageProperties &properties) const;

		/**
		 * @brief Copies data from one Vulkan buffer to another.
		 *
		 * This function is responsible for copying data from a source Vulkan
		 * buffer to a destination Vulkan buffer based on the provided
		 * properties, which include the logical device, command pool, graphics
		 * queue, source buffer destination buffer, and the size of the data to
		 * copy. The function will record and submit a command buffer that
		 * performs the copy operation, ensuring that the data is transferred
		 * correctly from the source buffer to the destination buffer.
		 *
		 * @param properties A structure containing all necessary parameters for
		 * copying data from one buffer to another.
		 *
		 * @throws std::runtime_error If the copy operation fails, an exception
		 * will be thrown with details about the failure.
		 */
		void copyBuffer(const CopyBufferProperties &properties) const;

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
									uint32_t mipLevels) const;

		/**
		 * The physical device, which represents a
		 * Vulkan-compatible GPU. It is used to query
		 * the capabilities of the GPU and to create
		 * logical devices for rendering operations.
		 */
		VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;

		/**
		 * The logical device, which represents an instance of a physical device
		 * and is used to perform rendering operations. It is created from a
		 * physical device and provides access to the GPU's resources and
		 * capabilities.
		 */
		VkDevice _device = VK_NULL_HANDLE;

		/**
		 * The Vulkan instance, which is the connection between the application
		 * and the Vulkan library. It is used to create and manage Vulkan
		 * resources and to query the capabilities of the Vulkan implementation.
		 */
		VkInstance _VkInstance = VK_NULL_HANDLE;

		protected:
		/**
		 * @brief Creates a Vulkan instance and initializes the device backend.
		 *
		 * This function is responsible for creating a Vulkan instance and
		 * performing any necessary initialization for the device backend. It
		 * takes the platform information, application name, and application
		 * version as parameters to set up the Vulkan instance with the
		 * appropriate application info and extensions.
		 *
		 * @param platform The platform information used to determine required
		 * Vulkan instance extensions.
		 * @param appName The name of the application, used in the Vulkan
		 * application info.
		 * @param appVersion The version of the application, used in the Vulkan
		 * application info.
		 *
		 * @throws std::runtime_error If instance creation fails or required
		 * extensions are not available.
		 *
		 * @note This function is pure virtual and must be implemented by
		 * derived classes to provide platform-specific Vulkan instance creation
		 * logic.
		 */
		virtual void createInstance(const IPlatform &platform,
									const std::string &appName,
									Version &appVersion) = 0;

		/**
		 * @brief Creates a Vulkan logical device and selects a physical device
		 * for rendering.
		 *
		 * This function is responsible for selecting a suitable Vulkan physical
		 * device (GPU) and creating a logical device that can be used for
		 * rendering operations. It typically involves querying the available
		 * physical devices, checking their capabilities, and creating a logical
		 * device with the necessary queues and features enabled.
		 *
		 * @throws std::runtime_error If no suitable physical device is found or
		 * if logical device creation fails.
		 *
		 * @note This function is pure virtual and must be implemented by
		 * derived classes to provide platform-specific logic for physical
		 * device selection and logical device creation.
		 */
		virtual void createLogicalDevice() = 0;

		/**
		 * @brief Selects a suitable Vulkan physical device for rendering.
		 *
		 * This function is responsible for picking a Vulkan physical device
		 * (GPU) that meets the requirements of the application. It typically
		 * involves enumerating the available physical devices, checking their
		 * properties and features, and selecting one that is compatible with
		 * the application's needs (e.g., supports required queue families,
		 * extensions, and features).
		 *
		 * @throws std::runtime_error If no suitable physical device is found.
		 *
		 * @note This function is pure virtual and must be implemented by
		 * derived classes to provide platform-specific logic for physical
		 * device selection.
		 */
		virtual void pickPhysicalDevice() = 0;

		/**
		 * @brief Retrieves the available Vulkan layers on the system.
		 *
		 * This function queries the Vulkan API to enumerate the available
		 * instance layers and returns a vector of VkLayerProperties structures
		 * containing information about each layer.
		 */
		std::vector<VkLayerProperties> getAvailableLayers();

		private:
		/**
		 * @brief Returns the given Vulkan format's component type.
		 *
		 * This function checks the provided Vulkan format and determines if it
		 * includes a stencil component. It returns true if the format has a
		 * stencil component, and false otherwise. This is useful for
		 * determining how to handle depth-stencil formats when creating image
		 * views or performing layout transitions.
		 *
		 * @param format The Vulkan format to check for a stencil component.
		 *
		 * @return bool True if the format includes a stencil component, false
		 * otherwise.
		 *
		 */
		bool hasStencilComponent(VkFormat format) const;

		/**
		 * @brief Finds a suitable memory type for Vulkan resource allocation.
		 *
		 * This function queries the Vulkan physical device's memory properties
		 * to find a memory type that matches the specified type filter and
		 * memory property flags. It iterates through the available memory types
		 * and checks if they meet the criteria defined by the type filter and
		 * properties. If a suitable memory type is found, its index is
		 * returned; otherwise an exception is thrown indicating that no
		 * suitable memory type was found.
		 *
		 * @param physicalDevice The Vulkan physical device whose memory
		 * properties will be queried.
		 * @param typeFilter A bitmask specifying the acceptable memory types
		 * based on the resource requirements (e.g., from memory requirements).
		 * @param properties A bitmask of VkMemoryPropertyFlags specifying the
		 * desired memory properties (e.g.,
		 * VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT).
		 *
		 * @return uint32_t The index of a suitable memory type that matches the
		 * type filter and properties.
		 *
		 * @note TODO: Remove useless properties (such as the logical device
		 * which is already a member of the class).
		 */
		uint32_t findMemoryType(uint32_t typeFilter,
								VkMemoryPropertyFlags properties) const;
	};
}	 // namespace evan