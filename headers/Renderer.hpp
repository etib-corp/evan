/*
** ETIB PROJECT, 2026
** evan
** File description:
** Renderer
*/

#pragma once

#include "ADeviceBackend.hpp"
#include "DeviceContext.hpp"
#include "ASwapchainContext.hpp"

#include "Frame.hpp"
#include "Shader.hpp"
#include "Vertex.hpp"

#include "Scene.hpp"

#include <fstream>
#include <algorithm>

namespace evan
{
	/**
	 * @brief The Renderer class is responsible for managing the graphics
	 * pipeline, descriptor sets, and rendering frames. It interacts with the
	 * device context and swapchain context to draw the scene on the screen.
	 *
	 * The Renderer class provides methods to create the graphics pipeline,
	 * descriptor set layout, and descriptor pool. It also handles the drawing
	 * of frames by recording command buffers and updating uniform buffers with
	 * scene data. The class maintains a collection of frames for rendering and
	 * manages the current frame index for synchronization.
	 *
	 * @note The Renderer class is designed to work with Vulkan and requires a
	 * valid DeviceContext and ASwapchainContext to function properly. It
	 * encapsulates the rendering logic and resource management for a
	 * Vulkan-based rendering system.
	 */
	class Renderer
	{
		public:
		/**
		 * @brief Constructs a Renderer object.
		 *
		 * This constructor initializes the Renderer by creating the graphics
		 * pipeline, descriptor set layout, and descriptor pool. It takes a
		 * reference to the DeviceContext, the Vulkan render pass, and the MSAA
		 * sample count as parameters. The constructor sets up the necessary
		 * resources for rendering frames and prepares the Renderer for drawing
		 * operations.
		 *
		 * @param deviceContext A reference to the DeviceContext, which provides
		 * access to the Vulkan device and related resources.
		 * @param renderPass The Vulkan render pass that defines the structure
		 * of the rendering operations.
		 * @param msaaSamples The MSAA sample count to be used for multisample
		 * anti aliasing operations in the graphics pipeline.
		 *
		 * @note The constructor may throw exceptions if resource creation
		 * fails, such as if the graphics pipeline or descriptor pool cannot be
		 * created successfully. Ensure that the provided DeviceContext and
		 * render pass are valid and properly initialized before constructing
		 * the Renderer.
		 */
		Renderer(DeviceContext &deviceContext, VkRenderPass renderPass,
				 VkSampleCountFlagBits msaaSamples);

		~Renderer();

		/**
		 * @brief Destroys the Renderer and releases associated resources.
		 *
		 * This method is responsible for cleaning up all Vulkan resources
		 * associated with the Renderer, including the graphics pipeline,
		 * pipeline layout, descriptor set layout, descriptor pool, and any
		 * frames that were created. It takes a Vulkan device as a parameter to
		 * perform the necessary cleanup operations. Implement this method to
		 * ensure that all resources related to the Renderer are properly
		 * released when the Renderer is no longer needed.
		 *
		 * @param device The Vulkan device used to perform cleanup operations
		 * for the Renderer.
		 *
		 * @note This method should be called when the Renderer is being
		 * destroyed to ensure that all Vulkan resources are properly released
		 * and to prevent memory leaks. Make sure to handle any exceptions that
		 * may arise during resource cleanup to maintain stability in the
		 * application.
		 */
		void destroy(VkDevice device);

		/**
		 * @brief Draws a frame by recording command buffers and submitting them
		 * to the graphics queue.
		 *
		 * This method handles the rendering of a frame by updating the uniform
		 * buffer with scene data, recording the command buffer with the
		 * appropriate render pass and framebuffer, and submitting the command
		 * buffer to the graphics queue for execution. It takes references to
		 * the DeviceContext, ASwapchainContext, and Scene as parameters to
		 * access the necessary resources and data for rendering. Implement this
		 * method to ensure that frames are rendered correctly based on the
		 * current scene and swapchain context.
		 *
		 * @param deviceContext A reference to the DeviceContext, which provides
		 * access to the Vulkan device and related resources.
		 * @param swapchainContext A reference to the ASwapchainContext, which
		 * provides access to the swapchain and render pass for rendering
		 * operations.
		 * @param scene A reference to the Scene, which contains the data to be
		 * rendered in the frame, including meshes, materials, and textures.
		 *
		 * @note This method should be called for each frame that needs to be
		 * rendered. Ensure that the DeviceContext, ASwapchainContext, and Scene
		 * are properly initialized and contain valid data before calling this
		 * method to avoid rendering issues or exceptions during command buffer
		 * recording and submission.
		 */
		void drawFrame(const DeviceContext &deviceContext,
					   ASwapchainContext &swapchainContext, const Scene &scene);

		/**
		 * @brief Creates a frame for rendering.
		 *
		 * This method initializes a Frame object by allocating the necessary
		 * Vulkan resources, such as command buffers and synchronization
		 * objects, for rendering operations. It takes a Vulkan command pool and
		 * a reference to the ADeviceBackend as parameters to set up the frame
		 * for rendering. Implement this method to ensure that frames are
		 * properly created and ready for use in the rendering process.
		 *
		 * @param commandPool The Vulkan command pool used to allocate command
		 * buffers for the frame.
		 * @param deviceBackend A reference to the ADeviceBackend that provides
		 * access to Vulkan resources and functions needed to create
		 * synchronization objects and uniform buffers for the frame.
		 *
		 * @note This method should be called during the initialization of the
		 * Renderer to create the necessary frames for rendering. Ensure that
		 * the command pool and device backend are properly initialized before
		 * calling this method to avoid issues during frame creation.
		 */
		void createFrame(VkCommandPool commandPool,
						 const ADeviceBackend &deviceBackend);

		/**
		 * @brief Retrieves the Vulkan descriptor pool associated with the
		 * Renderer.
		 *
		 * This method returns the Vulkan descriptor pool that is used by the
		 * Renderer to allocate descriptor sets for rendering operations. The
		 * descriptor pool is a crucial resource for managing descriptor sets,
		 * which are used to bind resources such as uniform buffers and textures
		 * to the graphics pipeline. Implement this method to provide access to
		 * the descriptor pool for other components of the rendering system that
		 * may need to allocate descriptor sets.
		 *
		 * @return The Vulkan descriptor pool associated with the Renderer.
		 */
		VkDescriptorPool getDescriptorPool() const;

		/**
		 * @brief Retrieves the Vulkan descriptor set layout associated with the
		 * Renderer.
		 *
		 * This method returns the Vulkan descriptor set layout that defines the
		 * structure of the descriptor sets used by the Renderer. The descriptor
		 * set layout specifies the types and counts of resources that can be
		 * bound to the graphics pipeline, such as uniform buffers and textures.
		 * Implement this method to provide access to the descriptor set layout
		 * for other components of the rendering system that may need to create
		 * or manage descriptor sets based on this layout.
		 *
		 * @param device The Vulkan device used to access the descriptor set
		 * layout.
		 *
		 * @return The Vulkan descriptor set layout associated with the
		 * Renderer.
		 */
		const std::vector<VkBuffer> getUniformBuffers() const;

		/**
		 * @brief Retrieves the Vulkan descriptor set layout associated with the
		 * Renderer.
		 *
		 * This method returns the Vulkan descriptor set layout that defines the
		 * structure of the descriptor sets used by the Renderer. The descriptor
		 * set layout specifies the types and counts of resources that can be
		 * bound to the graphics pipeline, such as uniform buffers and textures.
		 * Implement this method to provide access to the descriptor set layout
		 * for other components of the rendering system that may need to create
		 * or manage descriptor sets based on this layout.
		 *
		 * @return The Vulkan descriptor set layout associated with the
		 * Renderer.
		 */
		VkDescriptorSetLayout getDescriptorSetLayout() const;

		protected:
		/**
		 * @brief Vulkan graphics pipeline used for rendering operations.
		 *
		 * This member variable holds the Vulkan graphics pipeline that is used
		 * by the Renderer to perform rendering operations. The graphics
		 * pipeline defines the stages of the rendering process, including
		 * vertex input, vertex shader, fragment shader, and output merger
		 * stages. It is a crucial resource for configuring how rendering
		 * commands are processed and how resources are bound to the pipeline
		 * during drawing operations.
		 */
		VkPipeline _pipeline;

		/**
		 * @brief Vulkan pipeline layout used for rendering operations.
		 *
		 * This member variable holds the Vulkan pipeline layout that is used by
		 * the Renderer to define the interface between shader stages and the
		 * resources bound to the graphics pipeline. The pipeline layout
		 * specifies the descriptor set layouts and push constant ranges that
		 * are used during rendering operations. It is essential for configuring
		 * how resources are accessed by shaders and how they are organized in
		 * the graphics pipeline.
		 */
		VkPipelineLayout _pipelineLayout;

		/**
		 * @brief A collection of frames used for rendering.
		 *
		 * This member variable holds a vector of Frame objects that are used by
		 * the Renderer to manage the rendering of frames. Each Frame contains
		 * the necessary Vulkan resources, such as command buffers and
		 * synchronization objects, for rendering operations. The Renderer uses
		 * this collection of frames to handle multiple frames in flight and to
		 * ensure proper synchronization between rendering operations.
		 */
		std::vector<Frame> _frames;

		/**
		 * @brief The index of the current frame being rendered.
		 *
		 * This member variable keeps track of the index of the current frame
		 * being rendered in the _frames vector. It is used to manage
		 * synchronization and to ensure that the correct frame resources are
		 * accessed during rendering operations. The Renderer updates this index
		 * as it cycles through the frames for rendering.
		 */
		uint32_t _currentFrameIndex;

		/**
		 * @brief Vulkan descriptor set layout used for rendering operations.
		 *
		 * This member variable holds the Vulkan descriptor set layout that
		 * defines the structure of the descriptor sets used by the Renderer.
		 * The descriptor set layout specifies the types and counts of resources
		 * that can be bound to the graphics pipeline, such as uniform buffers
		 * and textures. It is essential for configuring how resources are
		 * accessed by shaders and how they are organized in the graphics
		 * pipeline during rendering operations.
		 */
		VkDescriptorSetLayout _descriptorSetLayout;

		/**
		 * @brief Vulkan descriptor pool used for allocating descriptor sets.
		 *
		 * This member variable holds the Vulkan descriptor pool that is used by
		 * the Renderer to allocate descriptor sets for rendering operations.
		 * The descriptor pool is a crucial resource for managing descriptor
		 * sets, which are used to bind resources such as uniform buffers and
		 * textures to the graphics pipeline. It is essential for ensuring that
		 * the Renderer can efficiently manage and allocate descriptor sets as
		 * needed during rendering operations.
		 */
		VkDescriptorPool _descriptorPool;

		private:
		/**
		 * @brief Updates the uniform buffer with scene data for the current
		 * frame.
		 *
		 * This method is responsible for updating the uniform buffer with the
		 * necessary data from the Scene for the current frame being rendered.
		 * It takes references to the Scene, ASwapchainContext, and the current
		 * frame index as parameters to access the relevant data and resources
		 * needed for updating the uniform buffer. Implement this method to
		 * ensure that the uniform buffer contains the correct data for
		 * rendering the scene in each frame.
		 */
		void updateUniformBuffer(const Scene &scene,
								 ASwapchainContext &swapchainContext,
								 int currentIndex);

		/**
		 * @brief Resets the command buffers for the current frame.
		 *
		 * This method is responsible for resetting the command buffers
		 * associated with the current frame being rendered. It ensures that the
		 * command buffers are in a clean state before recording new commands
		 * for the next frame. Implement this method to properly manage command
		 * buffer states and to avoid issues during command buffer recording and
		 * submission in the rendering process.
		 */
		void resetCommandBuffers();

		/**
		 * @brief Records the command buffer for rendering a frame.
		 *
		 * This method is responsible for recording the command buffer with the
		 * necessary commands to render a frame based on the provided render
		 * pass, framebuffer, swap chain extent, and scene data. It takes
		 * references to the render pass, framebuffer, swap chain extent, and
		 * scene as parameters to access the relevant resources and data needed
		 * for recording the command buffer. Implement this method to ensure
		 * that the command buffer contains the correct commands for rendering
		 * the scene in each frame.
		 */
		void recordCommandBuffer(VkRenderPass renderPass,
								 VkFramebuffer swapChainFramebuffer,
								 VkExtent2D swapChainExtent,
								 const Scene &scene);

		/**
		 * @brief Creates the Vulkan descriptor set layout for rendering
		 * operations.
		 *
		 * This method initializes the Vulkan descriptor set layout that defines
		 * the structure of the descriptor sets used by the Renderer. The
		 * descriptor set layout specifies the types and counts of resources
		 * that can be bound to the graphics pipeline, such as uniform buffers
		 * and textures. Implement this method to ensure that the Renderer has a
		 * properly configured descriptor set layout for managing resources
		 * during rendering operations.
		 */
		void createDescriptorSetLayout(VkDevice device);

		/**
		 * @brief Creates the Vulkan graphics pipeline for rendering operations.
		 *
		 * This method initializes the Vulkan graphics pipeline that is used by
		 * the Renderer to perform rendering operations. The graphics pipeline
		 * defines the stages of the rendering process, including vertex input,
		 * vertex shader, fragment shader, and output merger stages. It takes
		 * references to the Vulkan device, render pass, and MSAA sample count
		 * as parameters to configure the graphics pipeline accordingly.
		 * Implement this method to ensure that the Renderer has a properly
		 * configured graphics pipeline for rendering scenes with the specified
		 * render pass and MSAA settings.
		 */
		void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass,
									VkSampleCountFlagBits msaaSamples);

		/**
		 * @brief Creates the Vulkan descriptor pool for allocating descriptor
		 * sets.
		 *
		 * This method initializes the Vulkan descriptor pool that is used by
		 * the Renderer to allocate descriptor sets for rendering operations.
		 * The descriptor pool is a crucial resource for managing descriptor
		 * sets, which are used to bind resources such as uniform buffers and
		 * textures to the graphics pipeline. It takes references to the Vulkan
		 * device and the material count as parameters to configure the
		 * descriptor pool accordingly. Implement this method to ensure that the
		 * Renderer has a properly configured descriptor pool for managing and
		 * allocating descriptor sets during rendering operations.
		 */
		void createDescriptorPool(VkDevice device, uint32_t materialCount);
	};
}	 // namespace evan
