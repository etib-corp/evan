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

namespace evan
{
	class ADeviceBackend
	{
		public:
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
		virtual std::vector<int64_t> enumerateSwapchainFormats(uint32_t swapchainFormatCount) const = 0;

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

		uint32_t findMemoryType(VkPhysicalDevice physicalDevice,
											   uint32_t typeFilter,
											   VkMemoryPropertyFlags properties) const;

		void createImage(const CreateImageProperties &properties) const;

		VkInstance
			_VkInstance;	/// The Vulkan instance, which is the connection
							/// between the application and the Vulkan library.
							/// It is used to create and manage Vulkan resources
							/// and to query the capabilities of the Vulkan
							/// implementation.
		VkPhysicalDevice
			_physicalDevice;	/// The physical device, which represents a
								/// Vulkan-compatible GPU. It is used to query
								/// the capabilities of the GPU and to create
								/// logical devices for rendering operations.
		VkDevice _device;		/// The logical device, which represents an
								/// instance of a physical device and is used to
								/// perform rendering operations.
		private:

	};
}	 // namespace evan