/*
** ETIB PROJECT, 2026
** evan
** File description:
** Frame
*/

#pragma once

#include "EvanPlatform.hpp"

#include "ADeviceBackend.hpp"

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
	class Frame
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
			glm::mat4 model;	// Model matrix
			glm::mat4 view;		// View matrix
			glm::mat4 proj;		// Projection matrix
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
		Frame(VkCommandPool commandPool, const ADeviceBackend &deviceBackend);

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
		 */
		VkBuffer getUniformBuffer() const;

		/**
		 * Fence to ensure that the frame is not rendered until the previous
		 * frame has finished.
		 */
		VkFence _inFlight;

		/**
		 * Semaphore to signal when the image is available for rendering.
		 */
		VkSemaphore _image;

		/**
		 * Semaphore to signal when rendering is finished and the image can be
		 * presented.
		 */
		VkCommandBuffer _commandBuffer;

		/**
		 * Semaphore to signal when rendering is finished and the image can be
		 * presented.
		 */
		VkSemaphore _render;

		/**
		 * @brief Pointer to the mapped memory of the uniform buffer.
		 */
		void *_uniformBufferMapped;

		protected:
		/**
		 * @brief Creates the Vulkan command buffer for this frame.
		 *
		 * This function allocates a command buffer from the specified
		 * command pool and initializes it for recording rendering commands.
		 */
		void createCommandBuffer(VkDevice device, VkCommandPool commandPool);

		/**
		 * @brief Creates the synchronization objects for this frame.
		 *
		 * This function creates the Vulkan synchronization objects (fences and
		 * semaphores) needed for rendering this frame, such as the in-flight
		 * fence, image available semaphore, and render finished semaphore.
		 */
		void createSyncObjects(VkDevice device);

		/**
		 * @brief Creates the uniform buffer for this frame.
		 *
		 * This function allocates a Vulkan buffer and its associated memory
		 * for the uniform buffer object (UBO) used in this frame.
		 * It sets up the buffer to be used for passing data to the vertex
		 * shader during rendering.
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
		VkBuffer _uniformBuffer;

		/**
		 * @brief Vulkan device memory associated with the uniform buffer.
		 *
		 * This memory is allocated to back the uniform buffer and is used to
		 * store the data for the UBO. It is mapped to allow CPU access for
		 * updating the UBO data before rendering.
		 */
		VkDeviceMemory _uniformBufferMemory;
	};
}	 // namespace evan
