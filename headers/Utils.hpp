/*
** ETIB PROJECT, 2025
** evan
** File description:
** Utils
*/

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <vector>

#include "EvanPlatform.hpp"

namespace evan
{
	class Utils
	{
		public:
		virtual ~Utils() = default;

		// Structs

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
		 * @brief A structure to represent Vulkan queue family indices.
		 *
		 * This structure is used to store the indices of queue families
		 * that support specific operations, such as graphics and presentation.
		 *
		 */
		struct QueueFamilyIndices {
			/*
			 * An optional value representing the index of the queue family
			 * that supports graphics operations.
			 */
			std::optional<uint32_t> graphicsFamily;

			/*
			 * An optional value representing the index of the queue family
			 * that supports presentation operations.
			 */
			std::optional<uint32_t> presentFamily;

			/*
			 * @brief isComplete
			 *
			 * A method to check if both graphicsFamily and presentFamily
			 * have been assigned values.
			 *
			 * @return true if both graphicsFamily and presentFamily have
			 * values, indicating that the required queue families are
			 * available. False otherwise.
			 */
			bool isComplete()
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		static std::vector<char> readFile(const std::string &filename);

		static SwapChainSupportDetails
			querySwapChainSupport(VkPhysicalDevice device,
								  VkSurfaceKHR surface);

		static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,
													VkSurfaceKHR surface);
		static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

		static uint32_t findMemoryType(VkPhysicalDevice physicalDevice,
									   uint32_t typeFilter,
									   VkMemoryPropertyFlags properties);
		static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

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

		static void createImage(const CreateImageProperties &properties);

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

		static void transitionImageLayout(
			const TransitionImageLayoutProperties &properties);

		static bool
			isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface,
							 std::vector<const char *> deviceExtensions);

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

		static void createBuffer(const CreateBufferProperties &properties);

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

		static void
			copyBufferToImage(const CopyBufferToImageProperties &properties);

		/**
		 * @struct GenerateMipmapsProperties
		 * @brief Holds properties required for generating mipmaps for a Vulkan
		 * image.
		 *
		 * This structure encapsulates all necessary Vulkan objects and image
		 * properties needed to perform mipmap generation operations.
		 *
		 */
		struct GenerateMipmapsProperties {
			/*
			 * @brief The Vulkan physical device used for querying image format
			 * support.
			 */
			VkPhysicalDevice _physicalDevice;
			/*
			 * @brief The Vulkan logical device used for command buffer
			 * operations.
			 */
			VkDevice _logicalDevice;
			/*
			 * @brief The Vulkan command pool used to allocate command buffers
			 * for mipmap generation.
			 */
			VkCommandPool _commandPool;
			/*
			 * @brief The Vulkan graphics queue used to submit the command
			 * buffer for execution.
			 */
			VkQueue _graphicsQueue;
			/*
			 * @brief The Vulkan image for which mipmaps are to be generated.
			 */
			VkImage _image;
			/*
			 * @brief The format of the image for which mipmaps are to be
			 * generated.
			 */
			VkFormat _imageFormat;
			/*
			 * @brief The width of the image in pixels.
			 */
			uint32_t _texWidth;
			/*
			 * @brief The height of the image in pixels.
			 */
			uint32_t _texHeight;
			/*
			 * @brief The number of mipmap levels to generate.
			 */
			uint32_t _mipLevels;
		};

		static void
			generateMipmaps(const GenerateMipmapsProperties &properties);

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

		static void copyBuffer(const CopyBufferProperties &properties);

		static VkSampleCountFlagBits
			getMaxUsableSampleCount(const VkPhysicalDevice &physicalDevice);

		static VkShaderModule createShaderModule(VkDevice logicalDevice,
												 const std::vector<char> &code);

		static void populateDebugMessengerCreateInfo(
			VkDebugUtilsMessengerCreateInfoEXT &createInfo,
			PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);

		static VkResult createDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
			const VkAllocationCallbacks *pAllocator,
			VkDebugUtilsMessengerEXT *pDebugMessenger);

		static VkFormat
			findSupportedDepthFormat(VkPhysicalDevice physicalDevice);
		static VkImageView createImageView(VkImage image, VkFormat format,
										   VkImageAspectFlags aspectFlags,
										   VkDevice logicalDevice,
										   uint32_t mipLevels);

		private:
		static VkFormat
			findSupportedFormat(VkPhysicalDevice physicalDevice,
								const std::vector<VkFormat> &candidates,
								VkImageTiling tiling,
								VkFormatFeatureFlags features);

		static bool hasStencilComponent(VkFormat format);

		static VkCommandBuffer
			beginSingleTimeCommands(VkDevice logicalDevice,
									VkCommandPool commandPool);
		static void endSingleTimeCommands(VkDevice logicalDevice,
										  VkCommandPool commandPool,
										  VkQueue graphicsQueue,
										  VkCommandBuffer commandBuffer);

		static bool checkDeviceExtensionSupport(
			VkPhysicalDevice device,
			std::vector<const char *> deviceExtensions);
	};
}	 // namespace evan

#endif /* !UTILS_HPP_ */
