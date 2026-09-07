/*
** ETIB PROJECT, 2026
** evan
** File description:
** Frame
*/

#pragma once

#include "evan/EvanPlatform.hpp"

#include "evan/ADeviceBackend.hpp"
#include "evan/DeviceContext.hpp"

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include <vector>

namespace evan
{
	/**
	 * @brief Frame class representing a single frame of rendering in the Vulkan
	 * application.
	 *
	 * This class encapsulates the Vulkan resources and synchronization objects
	 * needed for rendering a single frame, such as command buffers, fences,
	 * semaphores, and uniform buffers. It provides functions to create and
	 * manage these resources, as well as to reset the command buffer for
	 * recording new commands each frame. The Frame class is designed to be used
	 * in a multi-frame rendering loop, where each frame has its own set of
	 * resources to ensure proper synchronization and efficient rendering.
	 *
	 * @note The Frame class is closely tied to the ADeviceBackend class, which
	 * provides access to Vulkan resources and functions needed for creating
	 * buffers and synchronization objects. It is important to ensure that the
	 * Frame class is properly integrated with the device backend to manage
	 * Vulkan resources effectively.
	 */
	class Frame:
		protected utility::logging::Loggable<Frame,
											 utility::logging::DefaultLogger>
	{
		public:
		/**
		 * @brief Uniform buffer object structure.
		 *
		 * This structure represents the data that will be passed to the
		 * vertex shader as a uniform buffer. It contains the model, view,
		 * and projection matrices used for rendering.
		 *
		 */
		struct UniformBufferObject {
			/**
			 * @brief Model matrix representing the transformation of the object
			 * in the scene. It is used to position, rotate, and scale the
			 * object in the world space.
			 */
			glm::mat4 model;

			/**
			 * @brief View matrix representing the camera's position and
			 * orientation in the scene. It is used to transform world
			 * coordinates into camera space for rendering.
			 */
			glm::mat4 view;

			/**
			 * @brief Projection matrix representing the perspective or
			 * orthographic projection used for rendering. It is used to
			 * transform camera space coordinates into clip space for rendering.
			 */
			glm::mat4 proj;	   // Projection matrix
		};

		/**
		 * @brief Constructs a Frame object.
		 *
		 * This constructor initializes the synchronization objects and command
		 * buffer for the frame. It takes a command pool and a reference to the
		 * device backend to set up the necessary Vulkan resources for
		 * rendering.
		 *
		 * @param commandPool The Vulkan command pool used to allocate command
		 * buffers for this frame.
		 * @param deviceBackend A reference to the device backend that provides
		 * access to Vulkan resources and functions needed to create
		 * synchronization objects and uniform buffers.
		 */
		Frame(std::shared_ptr<DeviceContext> deviceContext);

		~Frame();

		/**
		 * @brief Destroys the Vulkan resources associated with this frame.
		 *
		 * This function cleans up the synchronization objects, command buffer,
		 * and uniform buffer associated with this frame. It should be called
		 * when the frame is no longer needed to free up Vulkan resources.
		 *
		 * @param device The Vulkan logical device used to destroy the
		 * resources.
		 */
		void destroy(VkDevice device);

		/**
		 * @brief Resets the command buffer for this frame.
		 *
		 * This function resets the command buffer to the initial state,
		 * allowing it to be recorded again for the next frame. It should be
		 * called at the beginning of each frame before recording commands.
		 */
		void resetCommandBuffer();

		/**
		 * @brief Gets the Vulkan command buffer associated with this frame.
		 *
		 * This function returns the command buffer that is used for recording
		 * rendering commands for this frame. It can be used to begin command
		 * buffer recording and submit commands to the graphics queue.
		 *
		 * @return The Vulkan command buffer for this frame.
		 */
		VkBuffer getUniformBuffer() const;

		/**
		 * Per-swapchain fences ensuring that a frame's submissions for each
		 * swapchain are not started until the previous frame using them has
		 * finished.
		 */
		std::vector<VkFence> _inFlight;

		/**
		 * Per-swapchain semaphores signaled when a swapchain image is
		 * available for rendering (only used by backends that signal them,
		 * e.g. desktop).
		 */
		std::vector<VkSemaphore> _imageAvailable;

		/**
		 * Vulkan command buffer recorded for this frame.
		 */
		VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;

		/**
		 * Per-swapchain semaphores signaled when rendering to a swapchain
		 * image is finished and the image can be presented.
		 */
		std::vector<VkSemaphore> _renderFinished;

		/**
		 * @brief Pointer to the mapped memory of the uniform buffer.
		 */
		void *_uniformBufferMapped = nullptr;

		protected:
		/**
		 * @brief Releases the synchronization objects, command buffer and
		 * uniform buffer owned by this frame.
		 *
		 * Idempotent: every handle is null-checked and reset after being
		 * destroyed, making it safe to call from both the destructor and the
		 * public destroy() method.
		 */
		void cleanup();
		/**
		 * @brief Creates the Vulkan command buffer for this frame.
		 *
		 * This function allocates a command buffer from the specified
		 * command pool and initializes it for recording rendering commands.
		 *
		 * @param device The Vulkan logical device used to create the command
		 * buffer.
		 * @param commandPool The Vulkan command pool from which to allocate the
		 */
		void createCommandBuffer(VkDevice device, VkCommandPool commandPool);

		/**
		 * @brief Creates the synchronization objects for this frame.
		 *
		 * This function creates the Vulkan synchronization objects (fences and
		 * semaphores) needed for rendering this frame, such as the in-flight
		 * fence, image available semaphore, and render finished semaphore.
		 *
		 * @param device The Vulkan logical device used to create the
		 * synchronization objects.
		 */
		void createSyncObjects(VkDevice device);

		/**
		 * @brief Creates the uniform buffer for this frame.
		 *
		 * This function allocates a Vulkan buffer and its associated memory
		 * for the uniform buffer object (UBO) used in this frame.
		 * It sets up the buffer to be used for passing data to the vertex
		 * shader during rendering.
		 *
		 * @param deviceBackend A reference to the device backend that provides
		 * access to Vulkan resources and functions needed to create the buffer
		 * and allocate memory for the uniform buffer.
		 */
		void createUniformBuffer(const ADeviceBackend &deviceBackend);

		/**
		 * @brief Vulkan buffer for the uniform buffer object (UBO) used in this
		 * frame.
		 *
		 * This buffer is allocated to store the model, view, and projection
		 * matrices that will be passed to the vertex shader as a uniform buffer
		 * during rendering.
		 */
		VkBuffer _uniformBuffer = VK_NULL_HANDLE;

		/**
		 * @brief Vulkan device memory associated with the uniform buffer.
		 *
		 * This memory is allocated to back the uniform buffer and is used to
		 * store the data for the UBO. It is mapped to allow CPU access for
		 * updating the UBO data before rendering.
		 */
		VkDeviceMemory _uniformBufferMemory = VK_NULL_HANDLE;

		/**
		 * @brief The device context used to create this frame, kept alive for
		 * the lifetime of the frame so that cleanup can safely destroy its
		 * Vulkan resources.
		 */
		std::shared_ptr<DeviceContext> _deviceContext;
	};
}	 // namespace evan
