/*
** ETIB PROJECT, 2025
** maverik
** File description:
** RenderingContext
*/

#ifndef RENDERINGCONTEXT_HPP_
#define RENDERINGCONTEXT_HPP_

#include "ARenderingContext.hpp"

#include "Vertex.hpp"

#include "Utils.hpp"

#include <map>

/*
 * @brief Maximum number of frames in flight.
 *
 * This constant defines the maximum number of frames that can be in flight
 * at any given time. It is used to manage synchronization and resource
 * allocation for rendering operations in Vulkan.
 *
 */
const int MAX_FRAMES_IN_FLIGHT = 2;

/*
 * @brief Maximum number of frames in flight.
 *
 * This constant defines the maximum number of frames that can be in flight
 * at any given time. It is used to manage synchronization and resource
 * allocation for rendering operations in Vulkan.
 *
 */
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, // Swap chain extension
    "VK_KHR_portability_subset"      // Portability subset extension
};

/*
 * @brief Validation layers for Vulkan debugging.
 *
 * This constant defines the validation layers used for debugging Vulkan
 * applications. These layers provide additional checks and validation
 * during development to help identify issues and improve code quality.
 *
 */
const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation" // Khronos validation layer
};

#ifdef NDEBUG
/**
 * @brief Flag to enable or disable validation layers.
 *
 * This constant determines whether validation layers are enabled
 * based on the build configuration. In release builds (NDEBUG defined),
 * validation layers are disabled for performance reasons. In debug builds,
 * validation layers are enabled to assist with debugging and development.
 */
const bool enableValidationLayers = false;
#else
/**
 * @brief Flag to enable or disable validation layers.
 *
 * This constant determines whether validation layers are enabled
 * based on the build configuration. In release builds (NDEBUG defined),
 * validation layers are disabled for performance reasons. In debug builds,
 * validation layers are enabled to assist with debugging and development.
 */
const bool enableValidationLayers = true;
#endif

namespace maverik {
namespace vk {
/**
 * @brief Rendering context class for Vulkan.
 *
 * This class represents a rendering context for Vulkan, providing methods
 * to initialize and manage Vulkan resources, including the window, surface,
 * physical and logical devices, command pools, buffers, and synchronization
 * objects. It inherits from the ARenderingContext abstract base class.
 *
 * The RenderingContext class is responsible for setting up the Vulkan
 * environment, selecting a suitable physical device, creating a logical device,
 * and managing various Vulkan resources required for rendering operations.
 * @note This class is designed to be extended and customized for specific
 * rendering applications. It provides a foundation for building Vulkan-based
 * rendering
 */
class RenderingContext : public ARenderingContext {
public:
  /**
   * @brief Window properties structure.
   *
   * This structure holds the properties of the window, including its width,
   * height, and title. It is used to initialize the window and set its
   * properties during the creation of the rendering context.
   *
   */
  struct WindowProperties {
    unsigned int width;  // Width of the window
    unsigned int height; // Height of the window
    std::string title;   // Title of the window
  };

  /**
   * @brief Constructs a RenderingContext with the specified window properties
   * and Vulkan instance.
   *
   * Initializes the rendering context using the provided window properties and
   * Vulkan instance handle.
   *
   * @param windowProperties The properties of the window to be used for
   * rendering.
   * @param instance The Vulkan instance to associate with this rendering
   * context.
   */
  RenderingContext(const WindowProperties &windowProperties,
                   VkInstance instance);

  // Destructor
  ~RenderingContext();

protected:
  GLFWwindow *_window;   // Pointer to the GLFW window
  VkSurfaceKHR _surface; // Vulkan surface for rendering
  VkQueue _presentQueue; // Vulkan queue for presentation

  std::vector<Vertex> _vertices;  // Vector of vertices for rendering
  std::vector<uint32_t> _indices; // Vector of indices for rendering

  /**
   * @brief Creates and initializes the vertex buffer required for rendering.
   *
   * This function allocates GPU memory and sets up the vertex buffer
   * to store vertex data used in rendering operations. It should be called
   * during the initialization phase before issuing any draw commands that
   * require vertex input.
   *
   * @note Ensure that the necessary Vulkan resources and device context
   *       are properly initialized before calling this function.
   */
  void createVertexBuffer();

  VkBuffer _vertexBuffer;             // Vulkan buffer for vertex data
  VkDeviceMemory _vertexBufferMemory; // Vulkan memory for vertex buffer

  VkBuffer _indexBuffer;             // Vulkan buffer for index data
  VkDeviceMemory _indexBufferMemory; // Vulkan memory for index buffer

  /**
   * @brief Creates and initializes the index buffer for rendering.
   *
   * This function allocates GPU memory and sets up the index buffer
   * required for indexed drawing operations. It typically uploads
   * index data to the buffer and prepares it for use in the rendering
   * pipeline.
   *
   * @note Must be called before issuing draw commands that use indexed
   * rendering.
   */
  void createIndexBuffer();

  std::vector<VkCommandBuffer>
      _commandBuffers; // Vector of Vulkan command buffers for rendering

  /**
   * @brief Allocates and records command buffers required for rendering
   * operations.
   *
   * This function creates the necessary Vulkan command buffers for the
   * rendering context. It typically allocates one command buffer per
   * framebuffer and records the commands needed to render each frame. The
   * command buffers are used during the rendering loop to submit drawing
   * commands to the GPU.
   *
   * @note This function should be called after the Vulkan device and swapchain
   * have been initialized. It may need to be called again if the swapchain is
   * recreated (e.g., on window resize).
   *
   * @throws std::runtime_error if command buffer allocation or recording fails.
   */
  void createCommandBuffers();

  std::vector<VkSemaphore>
      _imageAvailableSemaphores; // Vector of Vulkan semaphores for image
                                 // availability
  std::vector<VkSemaphore>
      _renderFinishedSemaphores; // Vector of Vulkan semaphores for rendering
                                 // completion
  std::vector<VkFence>
      _inFlightFences; // Vector of Vulkan fences for synchronization

  /**
   * @brief Creates synchronization objects required for rendering operations.
   *
   * This function initializes synchronization primitives such as semaphores and
   * fences, which are used to coordinate rendering and presentation between the
   * CPU and GPU. It should be called during the setup phase of the rendering
   * context.
   *
   * @note Must be called before starting the rendering loop.
   * @throws std::runtime_error if synchronization object creation fails.
   */
  void createSyncObjects();

  /**
   * @brief Initializes the application window with the specified dimensions and
   * title.
   *
   * This function sets up a window for rendering, using the provided width,
   * height, and title. It should be called before any rendering operations are
   * performed.
   *
   * @param width The width of the window in pixels.
   * @param height The height of the window in pixels.
   * @param title The title of the window.
   */
  void initWindow(unsigned int width, unsigned int height,
                  const std::string &title);

  /**
   * @brief Creates a Vulkan surface for rendering.
   *
   * This function initializes the Vulkan surface associated with the given
   * VkInstance. The created surface is typically used for presenting rendered
   * images to a window or display.
   *
   * @param instance The Vulkan instance with which to create the surface.
   *
   * @note The specific platform-dependent details (such as window handles)
   * required for surface creation should be handled internally or provided
   * elsewhere in the class.
   * @throws std::runtime_error If surface creation fails.
   */
  void createSurface(VkInstance instance);

  /**
   * @brief Selects and assigns a suitable physical device (GPU) for Vulkan
   * operations.
   *
   * This method scans available physical devices on the system and picks one
   * that meets the application's requirements. The selected device will be used
   * for all subsequent Vulkan operations.
   *
   * @param instance The Vulkan instance used to enumerate physical devices.
   */
  void pickPhysicalDevice(VkInstance instance) override;

  /**
   * @brief Creates and initializes the logical device for the rendering
   * context.
   *
   * This method sets up the logical device, which is required for issuing
   * rendering commands and managing resources in the Vulkan API. It should be
   * called after selecting the physical device and before performing any
   * rendering operations.
   *
   * @throws std::runtime_error If the logical device creation fails.
   */
  void createLogicalDevice() override;

  /**
   * @brief Creates the Vulkan command pool for command buffer allocation.
   *
   * This method initializes the command pool required for recording and
   * submitting command buffers to the GPU. It should be called during the
   * rendering context setup phase. The implementation may configure the command
   * pool with appropriate flags and associate it with the correct queue family
   * index.
   *
   * @note This method overrides a virtual function from the base class.
   *
   * @throws std::runtime_error If the command pool creation fails.
   */
  void createCommandPool() override;

private:
  /**
   * @brief Retrieves the maximum usable sample count for multisampling.
   *
   * This function queries the physical device's properties to determine the
   * highest supported sample count that can be used for multisample
   * anti-aliasing (MSAA).
   *
   * @return VkSampleCountFlagBits The maximum supported sample count for MSAA.
   */
  VkSampleCountFlagBits getMaxUsableSampleCount();
};
} // namespace vk
} // namespace maverik

#endif /* !RENDERINGCONTEXT_HPP_ */
