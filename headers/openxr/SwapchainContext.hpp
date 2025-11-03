/*
** ETIB PROJECT, 2025
** evan
** File description:
** SwapchainContext
*/

#ifndef SWAPCHAINCONTEXT_HPP_
#define SWAPCHAINCONTEXT_HPP_

#include <algorithm>
#include <array>
#include <map>
#include <string>

#include "ASwapchainContext.hpp"
#include "EvanPlatform.hpp"
#include "Utils.hpp"

namespace evan {
namespace openxr {

/**
 * @brief Properties required to create a SwapchainContext for OpenXR
 *
 * @param _instance The OpenXR instance
 * @param _systemId The OpenXR system ID
 * @param _session The OpenXR session
 * @param _physicalDevice The Vulkan physical device
 * @param _device The Vulkan logical device
 * @param _msaaSamples The number of samples for MSAA (default is VK
 * _SAMPLE_COUNT_1_BIT)
 * @param _commandPool The Vulkan command pool
 * @param _graphicsQueue The Vulkan graphics queue
 */
struct SwapchainContextCreationPropertiesXR {
  XrInstance _instance;
  XrSystemId _systemId;
  XrSession _session;
  VkPhysicalDevice _physicalDevice;
  VkDevice _device;
  VkSampleCountFlagBits _msaaSamples = VK_SAMPLE_COUNT_1_BIT;
  VkCommandPool _commandPool;
  VkQueue _graphicsQueue;
};

/**
 * @brief Properties required to create SwapchainImages for OpenXR
 *
 * @param _device The Vulkan logical device
 * @param _physicalDevice The Vulkan physical device
 * @param _capacity The number of images in the swapchain
 * @param _swapchainCreateInfo The OpenXR swapchain creation info
 * @param _commandPool The Vulkan command pool
 * @param _graphicsQueue The Vulkan graphics queue
 */
struct SwapchainImageCreationPropertiesXR {
  VkDevice _device;
  VkPhysicalDevice _physicalDevice;
  uint32_t _capacity;
  XrSwapchainCreateInfo _swapchainCreateInfo;
  VkCommandPool _commandPool;
  VkQueue _graphicsQueue;
};

/**
 * @brief Structure representing a swapchain image and its associated resources.
 *
 * This structure encapsulates the Vulkan images, image views, framebuffers,
 * command buffers, and synchronization primitives required for rendering to a
 * swapchain in an OpenXR application.
 *
 * It includes methods for initializing the resources, creating color and depth
 * images, creating framebuffers, and retrieving the first image pointer.
 */
struct SwapChainImage {

  VkFormat _swapchainImageFormat =
      VK_FORMAT_UNDEFINED;     // Format of the swapchain images
  VkExtent2D _swapchainExtent; // Dimensions of the swapchain images
  std::vector<XrSwapchainImageVulkan2KHR>
      _swapchainImages; // Vulkan images in the swapchain
  std::vector<VkImageView>
      _swapchainImageViews; // Image views for the swapchain images

  std::vector<VkFramebuffer>
      _swapchainFrameBuffers; // Framebuffers for rendering to the swapchain
                              // images

  VkImage _colorImage = VK_NULL_HANDLE; // Multisampled color image for MSAA
  VkDeviceMemory _colorImageMemory =
      VK_NULL_HANDLE; // Memory for the multisampled color image
  VkImageView _colorImageView =
      VK_NULL_HANDLE; // Image view for the multisampled color image

  VkImage _depthImage = VK_NULL_HANDLE; // Depth image for the swapchain
  VkDeviceMemory _depthImageMemory =
      VK_NULL_HANDLE; // Memory for the depth image
  VkImageView _depthImageView =
      VK_NULL_HANDLE; // Image view for the depth image

  std::vector<VkCommandBuffer>
      _graphicsCommandBuffers; // Command buffers for rendering commands
  const uint32_t _maxFramesInFlight =
      2; // Maximum number of frames that can be processed concurrently

  std::vector<VkFence> _inFlightFences; // Fences to ensure that a frame has
                                        // finished before starting a new one
  std::vector<VkFence>
      _imagesInFlight; // Fences to ensure that an image is not being used by
                       // multiple frames simultaneously

  XrSwapchainImageBaseHeader
      *_image; // Pointer to the first image in the swapchain

  uint32_t _currentFame = 0; // Index of the current frame being processed

  VkViewport _viewport = {0, 0, 0, 0, 0, 1.0}; // Viewport for rendering
  VkRect2D _scissor = {{0, 0}, {0, 0}}; // Scissor rectangle for rendering

  VkDevice _device;                 // Vulkan logical device
  VkPhysicalDevice _physicalDevice; // Vulkan physical device

  VkCommandPool _commandPool; // Vulkan command pool
  VkQueue _graphicsQueue;     // Vulkan graphics queue

  /**
   * @brief Initializes the swapchain image resources.
   *
   * @param properties Properties required for creating swapchain images.
   *
   * @note This function must be called before using the swapchain images for
   * rendering.
   */
  void createColorResources();

  /**
   * @brief Creates the depth resources for the swapchain images.
   *
   * @note This function must be called after creating the color resources.
   */
  void createDepthResources();

  /**
   * @brief Creates the framebuffers for the swapchain images.
   *
   * @note This function must be called after creating the color and depth
   * resources.
   */
  void createFrameBuffers();

  // void createCommandBuffers();

  /**
   * @brief Initializes the swapchain image with the given properties.
   *
   * @param properties Properties required for creating swapchain images.
   *
   * @note This function must be called before using the swapchain images for
   * rendering.
   */
  void init(const SwapchainImageCreationPropertiesXR &properties);

  /**
   * @brief Cleans up and releases all resources associated with the swapchain
   * image.
   *
   * @note This function should be called when the swapchain image is no longer
   * needed.
   */
  XrSwapchainImageBaseHeader *getFirstImagePointer();

  bool _initialized =
      false; // Flag indicating whether the swapchain image has been initialized
};

/**
 * @brief Swapchain context for managing swapchains in an OpenXR application.
 *
 * This class extends the ASwapchainContext base class and provides
 * functionality for creating and managing swapchains, render passes, and
 * graphics pipelines specific to OpenXR applications using Vulkan.
 *
 * It includes methods for initializing the swapchain context, selecting
 * swapchain formats, creating render passes, and creating graphics pipelines.
 */
class SwapchainContext : public ASwapchainContext {
public:
  /**
   * @brief Constructs a SwapchainContext with the given properties.
   *
   * @param properties Properties required for creating the swapchain context.
   *
   * @note This constructor initializes the swapchain context and prepares it
   * for rendering.
   */
  SwapchainContext(const SwapchainContextCreationPropertiesXR &properties);

  // Destructor
  ~SwapchainContext();

protected:
  /**
   * @brief Initializes the swapchain context.
   *
   * @note This function must be called before using the swapchain context for
   * rendering.
   */
  void init();

  // void initSwapchainImages();

  /**
   * @brief Selects the most suitable swapchain format from the available
   * formats.
   *
   * @param swapchainFormats A vector of available swapchain formats.
   *
   * @return The selected swapchain format.
   */
  VkFormat selectSwapchainFormat(const std::vector<int64_t> &swapchainFormats);

  /**
   * @brief Creates a swapchain image for the given swapchain.
   *
   * @param swapchainCreateInfo The OpenXR swapchain creation info.
   * @param swapchain The ASwapchain object to associate with the created
   * swapchain image
   *
   * @return A shared pointer to the created SwapchainImage.
   */
  std::shared_ptr<SwapChainImage>
  createSwapchainImage(XrSwapchainCreateInfo &swapchainCreateInfo,
                       ASwapchain<XrSwapchain> &swapchain);

  /**
   * @brief Creates and initializes the render pass for the swapchain context.
   *
   * This function sets up the render pass, which defines how rendering
   * operations are performed and how framebuffers are used within the swapchain
   * context.
   *
   * @note This function should be called before starting any rendering
   * operations that depend on the render pass configuration.
   */
  void createRenderPass() override;

  /**
   * @brief Creates and initializes the graphics pipeline for rendering
   * operations.
   *
   * This function sets up the graphics pipeline, including shader stages,
   * fixed-function state, and any other necessary pipeline configurations
   * required for rendering.
   *
   * @note This function should be called before issuing any draw commands.
   */
  void createGraphicsPipeline() override;

private:
  XrInstance _instance;             // The OpenXR instance
  XrSystemId _systemId;             // The OpenXR system ID
  XrSession _session;               // The OpenXR session
  VkPhysicalDevice _physicalDevice; // The Vulkan physical device
  VkDevice _device;                 // The Vulkan logical device
  VkSampleCountFlagBits _msaaSamples =
      VK_SAMPLE_COUNT_1_BIT;  // MSAA sample count
  VkCommandPool _commandPool; // The Vulkan command pool
  VkQueue _graphicsQueue;     // The Vulkan graphics queue

  std::vector<XrViewConfigurationView>
      _viewsConfigurations;   // View configurations for the swapchain
  std::vector<XrView> _views; // Views for the swapchain

  std::map<XrSwapchain, std::shared_ptr<SwapChainImage>>
      _swapchainImages; // Map of swapchain handles to their corresponding
                        // SwapchainImage objects
};

} // namespace openxr
} // namespace evan

#endif /* !SWAPCHAINCONTEXT_HPP_ */
