/*
** ETIB PROJECT, 2025
** maverik
** File description:
** Utils
*/

#include "Utils.hpp"

/**
 * @brief Reads the contents of a binary file into a vector of characters.
 *
 * This function opens the specified file in binary mode, reads its entire
 * contents into a buffer, and returns the buffer as a std::vector<char>.
 *
 * @param filename The path to the file to be read.
 * @return A std::vector<char> containing the contents of the file.
 *
 * @throws std::runtime_error If the file cannot be opened.
 */
std::vector<char> maverik::Utils::readFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();
  return buffer;
}

/**
 * @brief Queries the swap chain support details for a given physical device and
 * surface.
 *
 * This function retrieves the capabilities, supported surface formats, and
 * present modes of the swap chain for the specified Vulkan physical device and
 * surface. The results are stored in a SwapChainSupportDetails structure.
 *
 * @param device The Vulkan physical device to query.
 * @param surface The Vulkan surface to query.
 * @return A SwapChainSupportDetails structure containing the swap chain support
 * details.
 *
 * The returned structure includes:
 * - Capabilities: The surface capabilities (e.g., min/max image count, current
 * extent, etc.).
 * - Formats: A list of supported surface formats (e.g., pixel format and color
 * space).
 * - Present Modes: A list of supported presentation modes (e.g., FIFO, Mailbox,
 * etc.).
 */
maverik::Utils::SwapChainSupportDetails
maverik::Utils::querySwapChainSupport(VkPhysicalDevice device,
                                      VkSurfaceKHR surface) {
  SwapChainSupportDetails details;
  uint32_t formatCount;
  uint32_t presentModeCount;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         details.formats.data());
  }

  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                            nullptr);
  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &presentModeCount, details.presentModes.data());
  }
  return details;
}

/**
 * @brief Finds the queue families that support specific operations on a given
 * Vulkan physical device and surface.
 *
 * This function identifies the queue families that support graphics operations
 * and presentation to a given surface. It iterates through the queue families
 * of the specified physical device and checks their capabilities.
 *
 * @param device The Vulkan physical device to query for queue family
 * properties.
 * @param surface The Vulkan surface to check for presentation support.
 * @return QueueFamilyIndices A structure containing the indices of the graphics
 * and presentation queue families. If no suitable queue families are found, the
 * indices will remain incomplete.
 */
maverik::Utils::QueueFamilyIndices
maverik::Utils::findQueueFamilies(VkPhysicalDevice device,
                                  VkSurfaceKHR surface) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

    if (presentSupport) {
      indices.presentFamily = i;
    }

    if (indices.isComplete()) {
      break;
    }

    i++;
  }

  return indices;
}

/**
 * @brief Finds the queue families supported by a given Vulkan physical device.
 *
 * This function iterates through the queue families of the specified physical
 * device and identifies the indices of queue families that support specific
 * capabilities, such as graphics operations.
 *
 * @param device The Vulkan physical device to query for queue family
 * properties.
 * @return A QueueFamilyIndices structure containing the indices of the queue
 * families that meet the required criteria. If no suitable queue families are
 * found, the indices will remain unset.
 */
maverik::Utils::QueueFamilyIndices
maverik::Utils::findQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
      break;
    }
    i++;
  }

  return indices;
}

/**
 * @brief Finds a suitable memory type for a Vulkan resource.
 *
 * This function searches through the memory types available on the given
 * physical device and returns the index of a memory type that satisfies the
 * specified type filter and memory property flags.
 *
 * @param physicalDevice The Vulkan physical device to query for memory
 * properties.
 * @param typeFilter A bitmask specifying the acceptable memory types. Each bit
 * represents a memory type, and the function will check which types are
 * suitable.
 * @param properties A set of memory property flags that the desired memory type
 * must have. For example, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT or
 *                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT.
 * @return The index of a suitable memory type.
 * @throws std::runtime_error If no suitable memory type is found.
 */
uint32_t maverik::Utils::findMemoryType(VkPhysicalDevice physicalDevice,
                                        uint32_t typeFilter,
                                        VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;

  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("Failed to find suitable memory type!");
}

/**
 * @brief Finds a suitable depth format for a Vulkan physical device.
 *
 * This function determines the most appropriate depth format supported by the
 * given Vulkan physical device. It checks a list of candidate formats in order
 * of preference and ensures that the selected format supports the specified
 * tiling and feature requirements.
 *
 * @param physicalDevice The Vulkan physical device to query for supported
 * formats.
 * @return VkFormat The selected depth format that meets the requirements.
 *
 * The function prioritizes the following formats (in order):
 * - VK_FORMAT_D32_SFLOAT
 * - VK_FORMAT_D32_SFLOAT_S8_UINT
 * - VK_FORMAT_D24_UNORM_S8_UINT
 *
 * The selected format must support:
 * - VK_IMAGE_TILING_OPTIMAL
 * - VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
 */
VkFormat maverik::Utils::findDepthFormat(VkPhysicalDevice physicalDevice) {
  return findSupportedFormat(
      physicalDevice,
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

/**
 * @brief Creates a Vulkan image and allocates memory for it.
 *
 * This function initializes a Vulkan image with the specified parameters,
 * allocates the required memory, and binds the memory to the image. It is a
 * utility function to simplify Vulkan image creation.
 *
 * @param logicalDevice The Vulkan logical device used to create the image.
 * @param physicalDevice The Vulkan physical device used to find memory
 * properties.
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param mipLevels The number of mipmap levels for the image.
 * @param numSamples The number of samples per pixel for multisampling.
 * @param format The format of the image (e.g., VK_FORMAT_R8G8B8A8_SRGB).
 * @param tiling The tiling arrangement of the image (e.g.,
 * VK_IMAGE_TILING_OPTIMAL).
 * @param usage The intended usage of the image (e.g.,
 * VK_IMAGE_USAGE_SAMPLED_BIT).
 * @param properties The memory properties required for the image (e.g.,
 * VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT).
 * @param image A reference to a VkImage handle where the created image will be
 * stored.
 * @param imageMemory A reference to a VkDeviceMemory handle where the allocated
 * memory will be stored.
 *
 * @throws std::runtime_error If the image creation or memory allocation fails.
 */
void maverik::Utils::createImage(const CreateImageProperties &properties) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = properties._width;
  imageInfo.extent.height = properties._height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = properties._format;
  imageInfo.tiling = properties._tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = properties._usage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.mipLevels = properties._mipLevels;
  imageInfo.samples = properties._numSamples;

  if (vkCreateImage(properties._logicalDevice, &imageInfo, nullptr,
                    &properties._image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(properties._logicalDevice, properties._image,
                               &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = Utils::findMemoryType(
      properties._physicalDevice, memRequirements.memoryTypeBits,
      properties._properties);

  if (vkAllocateMemory(properties._logicalDevice, &allocInfo, nullptr,
                       &properties._imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate image memory!");
  }

  vkBindImageMemory(properties._logicalDevice, properties._image,
                    properties._imageMemory, 0);
}

/**
 * @brief Transitions the layout of a Vulkan image.
 *
 * This function is used to transition a Vulkan image from one layout to
 * another. It sets up the necessary image memory barriers and pipeline stages
 * to ensure proper synchronization during the transition.
 *
 * @param logicalDevice The Vulkan logical device.
 * @param commandPool The command pool used to allocate the command buffer.
 * @param graphicsQueue The graphics queue used to submit the command buffer.
 * @param image The Vulkan image to transition.
 * @param format The format of the image, used to determine if it has a stencil
 * component.
 * @param oldLayout The current layout of the image.
 * @param newLayout The desired layout of the image.
 * @param mipLevels The number of mipmap levels in the image.
 *
 * @throws std::invalid_argument If the layout transition is unsupported.
 *
 * @note This function assumes that the image is not being used concurrently
 *       by other operations during the transition.
 */
void maverik::Utils::transitionImageLayout(
    const TransitionImageLayoutProperties &properties) {
  VkCommandBuffer commandBuffer = Utils::beginSingleTimeCommands(
      properties._logicalDevice, properties._commandPool);

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = properties._oldLayout;
  barrier.newLayout = properties._newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = properties._image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = properties._mipLevels;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (properties._oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      properties._newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (properties._oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             properties._newLayout ==
                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (properties._oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             properties._newLayout ==
                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  if (properties._newLayout ==
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (Utils::hasStencilComponent(properties._format)) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);

  Utils::endSingleTimeCommands(properties._logicalDevice,
                               properties._commandPool,
                               properties._graphicsQueue, commandBuffer);
}

/**
 * @brief Checks if a Vulkan physical device is suitable for use.
 *
 * This function evaluates whether a given Vulkan physical device meets the
 * requirements for the application, including queue family support, device
 * extension support, swap chain adequacy, and specific device features.
 *
 * @param device The Vulkan physical device to evaluate.
 * @param surface The Vulkan surface associated with the application.
 * @param deviceExtensions A list of required device extensions.
 * @return true If the device is suitable for use.
 * @return false If the device does not meet the requirements.
 *
 * The function performs the following checks:
 * - Verifies that the device has the necessary queue families.
 * - Ensures that all required device extensions are supported.
 * - Checks that the swap chain is adequate (has at least one format and one
 * present mode).
 * - Confirms that the device supports anisotropic sampling.
 */
bool maverik::Utils::isDeviceSuitable(
    VkPhysicalDevice device, VkSurfaceKHR surface,
    std::vector<const char *> deviceExtensions) {
  QueueFamilyIndices indices = Utils::findQueueFamilies(device, surface);
  bool extensionsSupported =
      Utils::checkDeviceExtensionSupport(device, deviceExtensions);
  bool swapChainAdequate = false;

  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport =
        Utils::querySwapChainSupport(device, surface);
    swapChainAdequate = !swapChainSupport.formats.empty() &&
                        !swapChainSupport.presentModes.empty();
  }
  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

  return indices.isComplete() && extensionsSupported && swapChainAdequate &&
         supportedFeatures.samplerAnisotropy;
}

/**
 * @brief Creates a Vulkan buffer and allocates memory for it.
 *
 * This function encapsulates the creation of a Vulkan buffer and the allocation
 * of its associated memory. It ensures that the buffer is properly created and
 * bound to the allocated memory.
 *
 * @param logicalDevice The Vulkan logical device used to create the buffer.
 * @param physicalDevice The Vulkan physical device used to find suitable memory
 * types.
 * @param size The size of the buffer in bytes.
 * @param usage A bitmask specifying allowed usages of the buffer (e.g., vertex
 * buffer, index buffer).
 * @param properties A bitmask specifying the memory properties required for the
 * buffer (e.g., host-visible, device-local).
 * @param buffer A reference to a VkBuffer handle where the created buffer will
 * be stored.
 * @param bufferMemory A reference to a VkDeviceMemory handle where the
 * allocated memory will be stored.
 *
 * @throws std::runtime_error If the buffer creation or memory allocation fails.
 */
void maverik::Utils::createBuffer(const CreateBufferProperties &properties) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = properties._size;
  bufferInfo.usage = properties._usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(properties._logicalDevice, &bufferInfo, nullptr,
                     &properties._buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(properties._logicalDevice, properties._buffer,
                                &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = Utils::findMemoryType(
      properties._physicalDevice, memRequirements.memoryTypeBits,
      properties._properties);

  if (vkAllocateMemory(properties._logicalDevice, &allocInfo, nullptr,
                       &properties._bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(properties._logicalDevice, properties._buffer,
                     properties._bufferMemory, 0);
}

/**
 * @brief Copies data from a Vulkan buffer to a Vulkan image.
 *
 * This function is used to transfer data from a buffer to an image in Vulkan.
 * It is typically used for uploading texture data to a GPU image resource.
 *
 * @param logicalDevice The Vulkan logical device used for the operation.
 * @param commandPool The command pool from which the command buffer is
 * allocated.
 * @param graphicsQueue The graphics queue used to submit the command buffer.
 * @param buffer The Vulkan buffer containing the data to be copied.
 * @param image The Vulkan image to which the data will be copied.
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 */
void maverik::Utils::copyBufferToImage(
    const CopyBufferToImageProperties &properties) {
  VkCommandBuffer commandBuffer = Utils::beginSingleTimeCommands(
      properties._logicalDevice, properties._commandPool);

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {properties._width, properties._height, 1};

  vkCmdCopyBufferToImage(commandBuffer, properties._buffer, properties._image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  Utils::endSingleTimeCommands(properties._logicalDevice,
                               properties._commandPool,
                               properties._graphicsQueue, commandBuffer);
}

/**
 * @brief Generates mipmaps for a given Vulkan image.
 *
 * This function creates mipmaps for a Vulkan image by performing linear
 * blitting between mip levels. It transitions image layouts and ensures proper
 * synchronization for each mip level. The function assumes that the image
 * format supports linear blitting.
 *
 * @param physicalDevice The Vulkan physical device used to query format
 * properties.
 * @param logicalDevice The Vulkan logical device used for command buffer
 * operations.
 * @param commandPool The command pool used to allocate the command buffer.
 * @param graphicsQueue The graphics queue used to submit the command buffer.
 * @param image The Vulkan image for which mipmaps are generated.
 * @param imageFormat The format of the Vulkan image.
 * @param texWidth The width of the base mip level of the image.
 * @param texHeight The height of the base mip level of the image.
 * @param mipLevels The total number of mip levels to generate.
 *
 * @throws std::runtime_error If the image format does not support linear
 * blitting.
 */
void maverik::Utils::generateMipmaps(
    const GenerateMipmapsProperties &properties) {
  VkFormatProperties formatProperties;
  vkGetPhysicalDeviceFormatProperties(
      properties._physicalDevice, properties._imageFormat, &formatProperties);

  if (!(formatProperties.optimalTilingFeatures &
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    throw std::runtime_error(
        "texture image format does not support linear blitting!");
  }

  VkCommandBuffer commandBuffer = Utils::beginSingleTimeCommands(
      properties._logicalDevice, properties._commandPool);

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = properties._image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  int32_t mipWidth = properties._texWidth;
  int32_t mipHeight = properties._texHeight;

  for (uint32_t i = 1; i < properties._mipLevels; i++) {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);

    VkImageBlit blit{};
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = i - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1,
                          mipHeight > 1 ? mipHeight / 2 : 1, 1};
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = i;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;

    vkCmdBlitImage(commandBuffer, properties._image,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, properties._image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                   VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &barrier);

    if (mipWidth > 1)
      mipWidth /= 2;
    if (mipHeight > 1)
      mipHeight /= 2;
  }

  barrier.subresourceRange.baseMipLevel = properties._mipLevels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);

  Utils::endSingleTimeCommands(properties._logicalDevice,
                               properties._commandPool,
                               properties._graphicsQueue, commandBuffer);
}

/**
 * @brief Copies data from one Vulkan buffer to another.
 *
 * This function performs a buffer-to-buffer copy operation using a single-time
 * command buffer. It is useful for transferring data between buffers, such as
 * staging buffer to a device-local buffer.
 *
 * @param logicalDevice The Vulkan logical device used to allocate and manage
 * resources.
 * @param commandPool The command pool from which the command buffer will be
 * allocated.
 * @param graphicsQueue The graphics queue used to submit the copy command.
 * @param srcBuffer The source buffer containing the data to be copied.
 * @param dstBuffer The destination buffer where the data will be copied to.
 * @param size The size of the data to copy, in bytes.
 */
void maverik::Utils::copyBuffer(const CopyBufferProperties &properties) {
  VkCommandBuffer commandBuffer = Utils::beginSingleTimeCommands(
      properties._logicalDevice, properties._commandPool);

  VkBufferCopy copyRegion{};
  copyRegion.size = properties._size;
  vkCmdCopyBuffer(commandBuffer, properties._srcBuffer, properties._dstBuffer,
                  1, &copyRegion);

  Utils::endSingleTimeCommands(properties._logicalDevice,
                               properties._commandPool,
                               properties._graphicsQueue, commandBuffer);
}

/**
 * @brief Determines the maximum usable sample count for multisampling supported
 * by the given physical device.
 *
 * This function queries the physical device properties and calculates the
 * highest sample count that is supported for both color and depth framebuffers.
 * It returns the largest VkSampleCountFlagBits value that is supported by the
 * device, which can be used for configuring multisampling in Vulkan.
 *
 * @param physicalDevice The Vulkan physical device to query for sample count
 * capabilities.
 * @return VkSampleCountFlagBits The maximum supported sample count for both
 * color and depth framebuffers.
 */
VkSampleCountFlagBits maverik::Utils::getMaxUsableSampleCount(
    const VkPhysicalDevice &physicalDevice) {
  VkPhysicalDeviceProperties physicalDeviceProperties;
  vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
  VkSampleCountFlags counts =
      physicalDeviceProperties.limits.framebufferColorSampleCounts &
      physicalDeviceProperties.limits.framebufferDepthSampleCounts;

  if (counts & VK_SAMPLE_COUNT_64_BIT)
    return VK_SAMPLE_COUNT_64_BIT;
  if (counts & VK_SAMPLE_COUNT_32_BIT)
    return VK_SAMPLE_COUNT_32_BIT;
  if (counts & VK_SAMPLE_COUNT_16_BIT)
    return VK_SAMPLE_COUNT_16_BIT;
  if (counts & VK_SAMPLE_COUNT_8_BIT)
    return VK_SAMPLE_COUNT_8_BIT;
  if (counts & VK_SAMPLE_COUNT_4_BIT)
    return VK_SAMPLE_COUNT_4_BIT;
  if (counts & VK_SAMPLE_COUNT_2_BIT)
    return VK_SAMPLE_COUNT_2_BIT;

  return VK_SAMPLE_COUNT_1_BIT;
}

/**
 * @brief Creates a Vulkan shader module from the given SPIR-V bytecode.
 *
 * @param logicalDevice The Vulkan logical device used to create the shader
 * module.
 * @param code A vector containing the SPIR-V bytecode for the shader.
 * @return VkShaderModule The created Vulkan shader module.
 *
 * @throws std::runtime_error If the shader module creation fails.
 *
 * This function wraps the Vulkan `vkCreateShaderModule` function to simplify
 * the creation of shader modules. The input SPIR-V bytecode is passed as a
 * vector of bytes, which is converted to the required format for Vulkan.
 */
VkShaderModule
maverik::Utils::createShaderModule(VkDevice logicalDevice,
                                   const std::vector<char> &code) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }

  return shaderModule;
}

/**
 * @brief Populates a VkDebugUtilsMessengerCreateInfoEXT structure with the
 * necessary information for creating a Vulkan debug messenger.
 *
 * @param createInfo Reference to a VkDebugUtilsMessengerCreateInfoEXT structure
 * that will be populated with the debug messenger creation details.
 * @param debugCallback A function pointer to the debug callback function that
 * will handle debug messages from the Vulkan validation layers.
 *
 * The populated structure includes:
 * - Message severity levels: Verbose, Warning, and Error.
 * - Message types: General, Validation, and Performance.
 * - The user-defined callback function for handling debug messages.
 */
void maverik::Utils::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo,
    PFN_vkDebugUtilsMessengerCallbackEXT debugCallback) {
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
}

/**
 * @brief Creates a debug messenger for Vulkan instance debugging.
 *
 * This function wraps the Vulkan function `vkCreateDebugUtilsMessengerEXT` to
 * create a debug messenger for capturing debug messages from the Vulkan
 * validation layers.
 *
 * @param instance The Vulkan instance to associate the debug messenger with.
 * @param pCreateInfo A pointer to a `VkDebugUtilsMessengerCreateInfoEXT`
 * structure specifying the details of the debug messenger to be created.
 * @param pAllocator A pointer to a `VkAllocationCallbacks` structure for custom
 * memory allocation callbacks, or `nullptr` to use the default allocator.
 * @param pDebugMessenger A pointer to a `VkDebugUtilsMessengerEXT` handle where
 * the created debug messenger will be stored.
 * @return `VK_SUCCESS` if the debug messenger was successfully created, or
 *         `VK_ERROR_EXTENSION_NOT_PRESENT` if the
 * `vkCreateDebugUtilsMessengerEXT` function is not available.
 *
 * @note This function requires the `VK_EXT_debug_utils` extension to be
 * enabled. Ensure that the extension is available and enabled in the Vulkan
 * instance.
 */
VkResult maverik::Utils::createDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

/**
 * @brief Finds and returns a supported depth/stencil format for the given
 * physical device.
 *
 * This function iterates through a list of candidate VkFormat values and checks
 * if each format supports usage as a depth/stencil attachment with the
 * specified image tiling (optimal). It queries the format properties of the
 * physical device and returns the first format that meets the required
 * features.
 *
 * @param physicalDevice The Vulkan physical device to query for supported
 * formats.
 * @return VkFormat The first supported format suitable for depth/stencil
 * attachment.
 * @throws std::runtime_error If no suitable format is found.
 */
VkFormat
maverik::Utils::findSupportedDepthFormat(VkPhysicalDevice physicalDevice) {
  const std::vector<VkFormat> candidates = {VK_FORMAT_D32_SFLOAT,
                                            VK_FORMAT_D32_SFLOAT_S8_UINT,
                                            VK_FORMAT_D24_UNORM_S8_UINT};
  VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
  VkFormatFeatureFlags features =
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

  for (auto format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}

/**
 * @brief Creates a Vulkan image view for a given image.
 *
 * This function sets up and creates a Vulkan image view, which is used to
 * describe how an image resource should be accessed. It specifies the format,
 * view type, and subresource range for the image view.
 *
 * @param image The Vulkan image for which the image view is created.
 * @param format The format of the image view (e.g., VK_FORMAT_R8G8B8A8_SRGB).
 * @param aspectFlags Specifies which aspect(s) of the image are included in the
 * view (e.g., VK_IMAGE_ASPECT_COLOR_BIT for color images).
 * @param logicalDevice The Vulkan logical device used to create the image view.
 *
 * @return A VkImageView handle representing the created image view.
 *
 * @throws std::runtime_error If the image view creation fails.
 */

VkImageView maverik::Utils::createImageView(VkImage image, VkFormat format,
                                            VkImageAspectFlags aspectFlags,
                                            VkDevice logicalDevice,
                                            uint32_t mipLevels) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;
  viewInfo.subresourceRange.levelCount = mipLevels;

  VkImageView imageView;
  if (vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }

  return imageView;
}

/////////////////////
// Private methods //
/////////////////////

/**
 * @brief Finds a supported Vulkan format from a list of candidates based on
 * specified tiling and feature requirements.
 *
 * This function iterates through a list of candidate formats and checks if any
 * of them meet the specified requirements for image tiling and format features.
 * If a suitable format is found, it is returned. If no suitable format is
 * found, an exception is thrown.
 *
 * @param physicalDevice The Vulkan physical device to query for format support.
 * @param candidates A list of candidate VkFormat values to check for support.
 * @param tiling The desired VkImageTiling (e.g., VK_IMAGE_TILING_LINEAR or
 * VK_IMAGE_TILING_OPTIMAL).
 * @param features The required VkFormatFeatureFlags that the format must
 * support.
 *
 * @return VkFormat The first format from the candidates list that meets the
 * specified requirements.
 *
 * @throws std::runtime_error If no supported format is found in the candidates
 * list.
 */
VkFormat maverik::Utils::findSupportedFormat(
    VkPhysicalDevice physicalDevice, const std::vector<VkFormat> &candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported format!");
}

/**
 * @brief Checks if the given Vulkan format includes a stencil component.
 *
 * This function determines whether the specified Vulkan format contains
 * a stencil component by comparing it against known formats that include
 * stencil data.
 *
 * @param format The Vulkan format to check (VkFormat).
 * @return true if the format includes a stencil component, false otherwise.
 */
bool maverik::Utils::hasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}

/**
 * @brief Begins a single-time command buffer for Vulkan operations.
 *
 * This function allocates and begins recording a command buffer that is
 * intended for short-lived operations, such as resource transfers or one-off
 * commands. The command buffer is allocated from the specified command pool and
 * is configured with the `VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT` flag,
 * indicating that it will be submitted only once before being reset or freed.
 *
 * @param logicalDevice The Vulkan logical device used to allocate the command
 * buffer.
 * @param commandPool The command pool from which the command buffer will be
 * allocated.
 * @return VkCommandBuffer The allocated and begun command buffer ready for
 * recording commands.
 *
 * @note The caller is responsible for ending the command buffer recording and
 * submitting it to a queue, as well as cleaning up resources after use.
 */
VkCommandBuffer
maverik::Utils::beginSingleTimeCommands(VkDevice logicalDevice,
                                        VkCommandPool commandPool) {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

/**
 * @brief Ends a single-time command buffer operation and cleans up resources.
 *
 * This function finalizes the execution of a single-time command buffer by
 * submitting it to the specified graphics queue, waiting for the queue to
 * become idle, and then freeing the command buffer resources.
 *
 * @param logicalDevice The Vulkan logical device used to free the command
 * buffer.
 * @param commandPool The command pool from which the command buffer was
 * allocated.
 * @param graphicsQueue The Vulkan queue to which the command buffer is
 * submitted.
 * @param commandBuffer The command buffer to be ended, submitted, and freed.
 */
void maverik::Utils::endSingleTimeCommands(VkDevice logicalDevice,
                                           VkCommandPool commandPool,
                                           VkQueue graphicsQueue,
                                           VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue);

  vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

/**
 * @brief Checks if a Vulkan physical device supports the required extensions.
 *
 * This function queries the available extensions for a given Vulkan physical
 * device and verifies if all the required extensions specified in the input
 * list are supported.
 *
 * @param device The Vulkan physical device to check for extension support.
 * @param deviceExtensions A vector of required device extension names as
 * C-style strings.
 * @return true if all required extensions are supported by the device, false
 * otherwise.
 */
bool maverik::Utils::checkDeviceExtensionSupport(
    VkPhysicalDevice device, std::vector<const char *> deviceExtensions) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}
