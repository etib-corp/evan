/*
** ETIB PROJECT, 2025
** evan
** File description:
** SwapchainContext
*/

#ifndef SWAPCHAINCONTEXT_HPP_
#define SWAPCHAINCONTEXT_HPP_

#include "ASwapchainContext.hpp"

#include "Vertex.hpp"

#include "Utils.hpp"

#include <map>

#ifndef MAX_FRAMES_IN_FLIGHT
	#define MAX_FRAMES_IN_FLIGHT 2
#endif

namespace evan
{
	namespace glfw
	{
		/**
		 * @class SwapchainContext
		 * @brief Manages a Vulkan swapchain context for rendering operations.
		 *
		 * This class extends the ASwapchainContext base class to provide a
		 * concrete implementation for managing a Vulkan swapchain, including
		 * image views, framebuffers, render passes, graphics pipelines, and
		 * texture images. It encapsulates the necessary Vulkan objects and
		 * operations required for rendering within a swapchain context.
		 *
		 * @note This class is specific to Vulkan and requires a valid Vulkan
		 * instance, physical device, logical device, and surface to function
		 * correctly.
		 */
		class SwapchainContext: public ASwapchainContext
		{
			public:
			/**
			 * @struct SwapchainContextCreationProperties
			 * @brief Holds the properties required to create a Vulkan swapchain
			 * context.
			 *
			 * This structure encapsulates all the necessary Vulkan and GLFW
			 * objects needed for initializing and managing a swapchain context,
			 * including surface, devices, window, MSAA settings, command pool,
			 * and graphics queue.
			 *
			 */
			struct SwapchainContextCreationProperties {
				/*
				 * @brief The Vulkan surface to present images to.
				 */
				VkSurfaceKHR _surface;
				/*
				 * @brief The Vulkan physical device (GPU) to use.
				 */
				VkPhysicalDevice _physicalDevice;
				/*
				 * @brief The Vulkan logical device associated with the physical
				 * device.
				 */
				VkDevice _logicalDevice;
				/*
				 * @brief A pointer to the GLFW window associated with the
				 * swapchain.
				 */
				GLFWwindow *_window;
				/*
				 * @brief The number of samples for multisample anti-aliasing
				 * (MSAA).
				 */
				VkSampleCountFlagBits _msaaSamples;
				/*
				 * @brief The Vulkan command pool used for command buffer
				 * allocation.
				 */
				VkCommandPool _commandPool;
				/*
				 * @brief The Vulkan graphics queue used for rendering commands.
				 */
				VkQueue _graphicsQueue;
				/*
				 * @brief The Vulkan instance associated with the swapchain
				 * context.
				 */
				VkInstance _instance;
			};

			/**
			 * @struct TextureImageCreationProperties
			 * @brief Holds Vulkan objects and properties required for texture
			 * image creation.
			 *
			 * This structure encapsulates the necessary Vulkan handles and
			 * settings needed to create and manage texture images, including
			 * the physical and logical devices, command pool, MSAA sample
			 * count, and graphics queue.
			 *
			 */
			struct TextureImageCreationProperties {
				/*
				 * @brief The Vulkan physical device used for resource creation.
				 */
				VkPhysicalDevice _physicalDevice;
				/*
				 * @brief The Vulkan logical device used for operations.
				 */
				VkDevice _logicalDevice;
				/*
				 * @brief The Vulkan command pool used for command buffer
				 * allocation.
				 */
				VkCommandPool _commandPool;
				/*
				 * @brief The number of samples for multisample anti-aliasing
				 * (MSAA).
				 */
				VkSampleCountFlagBits _msaaSamples;
				/*
				 * @brief The Vulkan graphics queue used for rendering
				 * operations.
				 */
				VkQueue _graphicsQueue;
			};

			/**
			 * @brief Uniform buffer object structure.
			 *
			 * This structure represents the data that will be passed to the
			 * vertex shader as a uniform buffer. It contains the model, view,
			 * and projection matrices used for rendering.
			 *
			 */
			struct UniformBufferObject {
				glm::mat4 model;	// Model matrix
				glm::mat4 view;		// View matrix
				glm::mat4 proj;		// Projection matrix
			};

			/**
			 * @brief Constructor for the SwapchainContext class.
			 *
			 * @param properties The properties required to create the swapchain
			 * context.
			 *
			 * @note This constructor initializes the swapchain context using
			 * the provided properties, setting up the necessary Vulkan objects
			 * and configurations.
			 */
			SwapchainContext(
				const SwapchainContextCreationProperties &properties);

			// Destructor
			~SwapchainContext();

			/**
			 * @brief Recreates the swapchain with new properties.
			 *
			 * @param properties The new properties for recreating the swapchain
			 * context.
			 *
			 * @note This function should be called when the swapchain needs to
			 * be recreated, such as when the window is resized or the surface
			 * format changes.
			 */
			void recreate(const SwapchainContextCreationProperties &properties);

			/**
			 * @brief Creates a texture image from the specified file path.
			 *
			 * @param texturePath The file path to the texture image.
			 * @param properties The properties required for texture image
			 * creation.
			 *
			 * @note This function loads the texture image from the specified
			 * path and creates the necessary Vulkan image resources for
			 * rendering.
			 */
			void createTextureImage(
				const std::string &texturePath,
				const TextureImageCreationProperties &properties);

			protected:
			std::vector<VkImage> _swapchainImages;	  // Images in the swapchain

			/**
			 * @brief Creates image views for the swapchain images.
			 *
			 * @param logicalDevice The Vulkan logical device used to create
			 * image views.
			 *
			 * @note This function initializes the image views for each image in
			 * the swapchain, which are used for rendering operations.
			 *
			 * @see createImageView
			 */
			void createImageViews(VkDevice logicalDevice);

			// Texture images
			std::map<std::string, VkImage>
				_textureImage;	  // Texture images mapped by their names
			VkDeviceMemory _textureImageMemory;	   // Memory for texture images
			std::map<std::string, VkImageView>
				_textureImageView;	  // Image views for texture images mapped
									  // by their names
			std::map<std::string, VkSampler>
				_textureSampler;	// Samplers for texture images mapped by
									// their names

			/**
			 * @brief Creates texture image views for all texture images.
			 *
			 * @param logicalDevice The Vulkan logical device used to create the
			 * image view.
			 *
			 * @note This function initializes the `_textureImageView` member by
			 * creating an image view for each texture image in the
			 * `_textureImage` map using the specified logical device. The image
			 * views are created with the `VK_FORMAT_R8G8B8A8_SRGB` format and
			 * the color aspect flag.
			 */
			void createTextureImageView(VkDevice logicalDevice);

			/**
			 * @brief Creates a Vulkan texture sampler for the swapchain
			 * context.
			 *
			 * This function initializes a texture sampler for the specified
			 * texture name using the provided logical device and physical
			 * device. The sampler is created with the given sampler creation
			 * info or defaults to standard settings if none are provided.
			 *
			 * @param logicalDevice The Vulkan logical device used to create the
			 * sampler.
			 * @param physicalDevice The Vulkan physical device used to query
			 * properties for sampler creation.
			 * @param textureName The name of the texture for which the sampler
			 * is being created.
			 * @param samplerInfo Optional sampler creation info; if not
			 * provided, default settings are used.
			 */
			void createTextureSampler(VkDevice logicalDevice,
									  VkPhysicalDevice physicalDevice,
									  const std::string &textureName,
									  VkSamplerCreateInfo samplerInfo = {});

			// Depth images
			VkImage _depthImage;				 // Depth image
			VkDeviceMemory _depthImageMemory;	 // Memory for depth image
			VkImageView _depthImageView;		 // Image view for depth image

			// Color images
			VkImage _colorImage;				 // Color image
			VkDeviceMemory _colorImageMemory;	 // Memory for color image
			VkImageView _colorImageView;		 // Image view for color image

			/**
			 * @brief Initializes the swapchain context with the specified
			 * properties.
			 *
			 * This function sets up the swapchain, image views, render pass,
			 * framebuffers, and graphics pipeline using the provided
			 * properties. It prepares the swapchain context for rendering
			 * operations.
			 *
			 * @param surface The Vulkan surface to present images to.
			 * @param physicalDevice The Vulkan physical device (GPU) to use.
			 * @param logicalDevice The Vulkan logical device associated with
			 * the physical device.
			 * @param window A pointer to the GLFW window associated with the
			 * swapchain.
			 */
			void init(VkSurfaceKHR surface, VkPhysicalDevice physicalDevice,
					  VkDevice logicalDevice, GLFWwindow *window);

			/**
			 * @brief Cleans up Vulkan resources associated with the swapchain
			 * context. This function destroys all framebuffers, image views,
			 * and the swapchain associated with the swapchain context. It
			 * ensures proper resource deallocation to prevent memory leaks.
			 *
			 * @param logicalDevice The Vulkan logical device used for resource
			 * cleanup.
			 *
			 * @note This function should be called before destroying the
			 * swapchain context.
			 */
			void cleanup(VkDevice logicalDevice);

			/**
			 * @brief Creates framebuffers for the swapchain images.
			 *
			 * This function initializes a framebuffer for each image view in
			 * the swapchain. Each framebuffer is configured with a color
			 * attachment, a depth attachment, and the corresponding swapchain
			 * image view. The framebuffers are stored in the
			 * `_swapchainFramebuffers` member variable.
			 *
			 * @param logicalDevice The Vulkan logical device used to create the
			 * framebuffers.
			 * @param renderPass The Vulkan render pass to be used with the
			 * framebuffers.
			 *
			 * @throws std::runtime_error If framebuffer creation fails.
			 */
			void createFramebuffers(VkDevice logicalDevice,
									VkRenderPass renderPass);

			VkPipelineLayout _pipelineLayout;	 // Vulkan pipeline layout
			VkPipeline _graphicsPipeline;		 // Vulkan graphics pipeline

			VkDescriptorPool _descriptorPool;	 // Vulkan descriptor pool for
												 // managing descriptor sets
			std::vector<VkDescriptorSet>
				_descriptorSets;	// Vector of Vulkan descriptor sets for
									// uniform data
			VkDescriptorSetLayout
				_descriptorSetLayout;	 // Vulkan descriptor set layout for
										 // uniform data
			VkDebugUtilsMessengerEXT
				_debugMessenger;	// Vulkan debug messenger for validation
									// layers

			/**
			 * @brief Sets up the Vulkan debug messenger for the swapchain
			 * context.
			 *
			 * This function initializes a Vulkan debug messenger if validation
			 * layers are enabled. The debug messenger is used to capture and
			 * handle debug messages from the Vulkan API, which can help in
			 * identifying issues during development.
			 *
			 * @param instance The Vulkan instance to associate the debug
			 * messenger with.
			 *
			 * @throws std::runtime_error If the debug messenger setup fails.
			 */
			void setupDebugMessenger(VkInstance instance);

			/**
			 * @brief Creates the descriptor set layout for the swapchain
			 * context.
			 *
			 * This function defines the layout of descriptor sets used in the
			 * graphics pipeline, specifying the types and counts of resources
			 * (uniform buffers, combined image samplers) that will be bound to
			 * the shaders.
			 *
			 * @param logicalDevice The Vulkan logical device used to create the
			 * descriptor set layout.
			 *
			 * @throws std::runtime_error If the Vulkan descriptor set layout
			 * creation fails.
			 */
			void createDescriptorSetLayout(VkDevice logicalDevice);

			/**
			 * @brief Creates a Vulkan descriptor pool for managing descriptor
			 * sets.
			 *
			 * This function initializes a descriptor pool with specific pool
			 * sizes for uniform buffers and combined image samplers. The number
			 * of descriptors allocated for each type is determined by the
			 * constant MAX_FRAMES_IN_FLIGHT.
			 *
			 * @throws std::runtime_error If the Vulkan descriptor pool creation
			 * fails.
			 *
			 * The descriptor pool is used to allocate descriptor sets, which
			 * are essential for binding resources (e.g., buffers and images) to
			 * shaders during rendering.
			 */
			void createDescriptorPool(VkDevice logicalDevice);

			/**
			 * @brief Creates and allocates descriptor sets for the swapchain
			 * context.
			 *
			 * This function allocates descriptor sets from the descriptor pool
			 * and configures them to bind uniform buffers and texture samplers
			 * for each frame in flight. The descriptor sets are stored in the
			 * `_descriptorSets` member variable.
			 *
			 * @param logicalDevice The Vulkan logical device used to allocate
			 * descriptor sets.
			 * @param imagesViewsAndSamplers A map of image views and their
			 * corresponding samplers to be bound.
			 *
			 * @throws std::runtime_error If the Vulkan descriptor set
			 * allocation or update fails.
			 *
			 * @note This function should be called after creating the
			 * descriptor pool and before rendering operations that require
			 * resource binding.
			 */
			void createDescriptorSets(
				VkDevice logicalDevice,
				std::map<VkImageView, VkSampler> imagesViewsAndSamplers);

			std::vector<VkBuffer>
				_uniformBuffers;	// Vector of Vulkan buffers for uniform data
			std::vector<VkDeviceMemory>
				_uniformBuffersMemory;	  // Vector of Vulkan memory for uniform
										  // buffers
			std::vector<void *>
				_uniformBuffersMapped;	  // Vector of mapped pointers to
										  // uniform buffer data

			/**
			 * @brief Creates uniform buffers for each frame in flight.
			 *
			 * This function initializes uniform buffers used to store uniform
			 * data (e.g., transformation matrices) for each frame in flight.
			 * The buffers are created with the appropriate size and usage
			 * flags, and their memory is allocated and mapped for CPU access.
			 *
			 * @param logicalDevice The Vulkan logical device used to create the
			 * uniform buffers.
			 * @param physicalDevice The Vulkan physical device used to query
			 * memory properties.
			 *
			 * @throws std::runtime_error If buffer creation or memory
			 * allocation fails.
			 *
			 * @note This function should be called before creating descriptor
			 * sets that bind the uniform buffers to shaders.
			 */
			void createUniformBuffers(VkDevice logicalDevice,
									  VkPhysicalDevice physicalDevice);

			// Inherited via ASwapchainContext
			void createRenderPass() override;

			// Inherited via ASwapchainContext
			void createGraphicsPipeline() override;

			private:
			SwapchainContextCreationProperties
				_creationProperties;	// Properties used for creating the
										// swapchain context

			/**
			 * @brief Chooses the best surface format from the available
			 * options.
			 *
			 * This function selects the most suitable surface format for the
			 * swapchain from the list of available formats. It prioritizes the
			 * VK_FORMAT_B8G8R8A8_SRGB format with
			 * VK_COLOR_SPACE_SRGB_NONLINEAR_KHR color space, but will fall back
			 * to the first available format if the preferred one is not found.
			 *
			 * @param availableFormats A vector of available VkSurfaceFormatKHR
			 * structures.
			 * @return VkSurfaceFormatKHR The chosen surface format for the
			 * swapchain.
			 */
			VkSurfaceFormatKHR chooseSwapSurfaceFormat(
				const std::vector<VkSurfaceFormatKHR> &availableFormats);

			/**
			 * @brief Chooses the best present mode from the available options.
			 *
			 * This function selects the most suitable present mode for the
			 * swapchain from the list of available present modes. It
			 * prioritizes the VK_PRESENT_MODE_MAILBOX_KHR mode for its low
			 * latency and ability to avoid tearing, but will fall back to
			 * VK_PRESENT_MODE_FIFO_KHR if the preferred mode is not found.
			 *
			 * @param availablePresentModes A vector of available
			 * VkPresentModeKHR values.
			 *
			 * @return VkPresentModeKHR The chosen present mode for the
			 * swapchain.
			 */
			VkPresentModeKHR chooseSwapPresentMode(
				const std::vector<VkPresentModeKHR> &availablePresentModes);

			/**
			 * @brief Chooses the swap extent (resolution) for the swapchain.
			 *
			 * This function determines the dimensions of the swapchain images
			 * based on the capabilities of the surface and the size of the GLFW
			 * window. It ensures that the chosen extent is within the allowed
			 * range specified by the surface capabilities.
			 *
			 * @param capabilities The VkSurfaceCapabilitiesKHR structure
			 * containing surface capabilities.
			 * @param window A pointer to the GLFW window used to determine the
			 * swap extent.
			 *
			 * @return VkExtent2D The chosen swap extent (width and height) for
			 * the swapchain images.
			 */
			VkExtent2D
				chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,
								 GLFWwindow *window);

			/**
			 * @brief Creates color resources for the swapchain, including a
			 * color image and its associated image view.
			 *
			 * This function initializes a color image with the specified
			 * format, extent, and sample count, and allocates memory for it.
			 * Additionally, it creates an image view for the color image to be
			 * used in rendering operations.
			 *
			 * @param logicalDevice The Vulkan logical device used to create the
			 * image and image view.
			 * @param physicalDevice The Vulkan physical device used to allocate
			 * memory for the image.
			 * @param msaaSamples The number of samples per pixel for
			 * multisampling (MSAA).
			 *
			 * @throws std::runtime_error If image or image view creation fails.
			 */
			void createColorResources(VkDevice logicalDevice,
									  VkPhysicalDevice physicalDevice,
									  VkSampleCountFlagBits msaaSamples);

			/**
			 * @brief Creates depth resources for the swapchain, including a
			 * depth image and its associated image view.
			 *
			 * This function initializes a depth image with the appropriate
			 * format, extent, and sample count, allocates memory for it, and
			 * creates an image view for the depth image to be used in rendering
			 * operations.
			 *
			 * @param properties The properties required for texture image
			 * creation, including physical and logical devices, command pool,
			 * MSAA sample count, and graphics queue.
			 *
			 * @throws std::runtime_error If image or image view creation fails.
			 */
			void createDepthResources(
				const TextureImageCreationProperties &properties);

			/**
			 * @brief Provides default sampler creation info based on physical
			 * device properties.
			 *
			 * This function generates a VkSamplerCreateInfo structure with
			 * default settings suitable for most texture sampling scenarios. It
			 * uses the provided physical device properties to set the maximum
			 * anisotropy level, ensuring optimal texture quality. The sampler
			 * is configured for linear filtering, repeat addressing mode, and
			 * standard mipmap settings.
			 *
			 * @param properties The VkPhysicalDeviceProperties structure
			 * containing properties of the physical device.
			 *
			 * @return VkSamplerCreateInfo A structure containing default
			 * sampler creation settings.
			 */
			VkSamplerCreateInfo getDefaultSamplerInfo(
				const VkPhysicalDeviceProperties &properties);

			/**
			 * @brief Creates and allocates a single descriptor set for the
			 * swapchain context.
			 *
			 * This function allocates a descriptor set from the descriptor pool
			 * and configures it to bind a uniform buffer and a texture sampler.
			 * The descriptor set is added to the `_descriptorSets` member
			 * variable.
			 *
			 * @param logicalDevice The Vulkan logical device used to allocate
			 * the descriptor set.
			 * @param textureImageView The Vulkan image view representing the
			 * texture to be sampled.
			 * @param textureSampler The Vulkan sampler used for sampling the
			 * texture.
			 *
			 * @throws std::runtime_error If the Vulkan descriptor set
			 * allocation or update fails.
			 *
			 * @note This function should be called after creating the
			 * descriptor pool and before rendering operations that require
			 * resource binding.
			 */
			void createSingleDescriptorSets(VkDevice logicalDevice,
											VkImageView textureImageView,
											VkSampler textureSampler);
		};
	}	 // namespace glfw
}	 // namespace evan

#endif /* !SWAPCHAINCONTEXT_HPP_ */
