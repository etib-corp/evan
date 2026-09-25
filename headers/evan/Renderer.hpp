/*
** ETIB PROJECT, 2026
** evan
** File description:
** Renderer
*/

#pragma once

#include "evan/ADeviceBackend.hpp"
#include "evan/DeviceContext.hpp"
#include "evan/ASwapchainContext.hpp"

#include "evan/Frame.hpp"

#include "evan/RenderSettings.hpp"
#include "evan/RessourceManager.hpp"

#include "evan/GPUShader.hpp"
#include "evan/GPUVertex.hpp"

#include "evan/Scene.hpp"

#include <utility/graphic/view.hpp>

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include <fstream>
#include <algorithm>
#include <cstdint>
#include <vector>

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
	class Renderer:
		protected utility::logging::Loggable<Renderer,
											 utility::logging::DefaultLogger>
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
		 * @param ressourceManager A shared pointer to the RessourceManager,
		 * which is responsible for managing materials, textures, and other
		 * resources used in rendering. The Renderer interacts with the
		 * RessourceManager to access the necessary resources for rendering
		 * scenes, such as materials and textures associated with render
		 * objects.
		 *
		 * @note The constructor may throw exceptions if resource creation
		 * fails, such as if the graphics pipeline or descriptor pool cannot be
		 * created successfully. Ensure that the provided DeviceContext and
		 * render pass are valid and properly initialized before constructing
		 * the Renderer.
		 */
		Renderer(std::shared_ptr<DeviceContext> deviceContext,
				 VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples,
				 std::shared_ptr<RessourceManager> ressourceManager);

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
		 * @return evan::Error::Ok on success. evan::Error::Suboptimal or
		 * evan::Error::SwapchainOutOfDate when the swapchain was recreated and
		 * the caller should retry on the next frame. Any other value is fatal
		 * (e.g. evan::Error::DeviceLost) and the caller should stop cleanly.
		 *
		 * @note This method should be called for each frame that needs to be
		 * rendered. Ensure that the DeviceContext, ASwapchainContext, and Scene
		 * are properly initialized and contain valid data before calling this
		 * method to avoid rendering issues or exceptions during command buffer
		 * recording and submission.
		 */
		Error drawFrame(const DeviceContext &deviceContext,
						ASwapchainContext &swapchainContext,
						const Scene &scene);

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
		void createFrame(std::shared_ptr<DeviceContext> deviceContext);

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

		/**
		 * @brief Enables or disables frustum and distance culling.
		 *
		 * When enabled, meshes whose bounds are fully outside the current
		 * view frustum (or beyond the maximum draw distance) are skipped
		 * before recording their draw commands. Disable for debugging: a
		 * wrong bound then never hides geometry.
		 *
		 * @param enabled True to cull, false to draw every mesh.
		 */
		void setCullingEnabled(bool enabled)
		{
			_cullingEnabled = enabled;
		}

		/**
		 * @brief Checks whether culling is currently enabled.
		 *
		 * @return True when culling is enabled.
		 */
		[[nodiscard]] bool isCullingEnabled() const
		{
			return _cullingEnabled;
		}

		/**
		 * @brief Sets the maximum draw distance for distance culling.
		 *
		 * Meshes whose bounding sphere is entirely beyond this distance from
		 * the view are culled. A value of 0 disables distance culling.
		 *
		 * @param maxDistance Maximum draw distance in world units (0 disables).
		 */
		void setMaxDrawDistance(float maxDistance)
		{
			_maxDrawDistance = maxDistance;
		}

		/**
		 * @brief Retrieves the maximum draw distance.
		 *
		 * @return The maximum draw distance (0 means disabled).
		 */
		[[nodiscard]] float getMaxDrawDistance() const
		{
			return _maxDrawDistance;
		}

		/**
		 * @brief Enables or disables instanced rendering.
		 *
		 * When enabled, consecutive meshes sharing the same geometry
		 * (pipeline, material, vertex and index buffers) are merged into a
		 * single vkCmdDrawIndexed with instanceCount > 1, and each mesh's
		 * transform is fed to the vertex shader from the per-frame instance
		 * buffer. The application must use per-instance transforms and a
		 * vertex shader that consumes the instance matrix for this to render
		 * correctly. Disabled by default.
		 *
		 * @param enabled True to batch identical meshes into instanced draws.
		 */
		void setInstancingEnabled(bool enabled)
		{
			_instancingEnabled = enabled;
		}

		/**
		 * @brief Checks whether instanced rendering is enabled.
		 *
		 * @return True when instancing is enabled.
		 */
		[[nodiscard]] bool isInstancingEnabled() const
		{
			return _instancingEnabled;
		}

		/**
		 * @brief Enables or disables indirect drawing.
		 *
		 * When enabled, the sorted draws are written into the per-frame
		 * indirect buffer and submitted with vkCmdDrawIndexedIndirect, one
		 * call per consecutive group sharing the same pipeline/descriptor/
		 * vertex state. Disabled by default.
		 *
		 * @param enabled True to batch draws through the indirect buffer.
		 */
		void setIndirectDrawingEnabled(bool enabled)
		{
			_indirectDrawingEnabled = enabled;
		}

		/**
		 * @brief Checks whether indirect drawing is enabled.
		 *
		 * @return True when indirect drawing is enabled.
		 */
		[[nodiscard]] bool isIndirectDrawingEnabled() const
		{
			return _indirectDrawingEnabled;
		}

		/**
		 * @brief Identifies one graphics pipeline variant.
		 *
		 * A variant is the pair of a shader and a blend mode: the opaque and
		 * the alpha-blended pipelines of a shader share their shader modules,
		 * layout and vertex input, and differ only in the fixed-function color
		 * blend and depth write state.
		 */
		struct PipelineVariant {
			uint32_t shaderID	= 0;
			BlendMode blendMode = BlendMode::Opaque;

			/**
			 * @brief Orders variants so they can be stored in a std::map.
			 *
			 * @param other The variant to compare against.
			 * @return True when this variant sorts before @p other.
			 */
			bool operator<(const PipelineVariant &other) const
			{
				if (shaderID != other.shaderID) {
					return shaderID < other.shaderID;
				}
				return static_cast<int>(blendMode)
					< static_cast<int>(other.blendMode);
			}
		};

		/**
		 * @brief Overrides the blend mode used for a shader.
		 *
		 * Materials decide their own blend mode (see
		 * utility::graphic::Material::getAlphaMode and
		 * GPUMaterial::getBlendMode), which covers assets that declare their
		 * transparency. This setter is the escape hatch for shaders whose alpha
		 * behaviour is not described by the material, for instance a shader
		 * that derives its transparency from something other than the albedo
		 * texture.
		 *
		 * The override applies to the next recorded frame; the pipelines of
		 * both variants already exist.
		 *
		 * @param shaderID The shader whose draws are switched.
		 * @param blendMode The blend mode used for every draw of that shader.
		 */
		void setShaderBlendMode(uint32_t shaderID, BlendMode blendMode);

		/**
		 * @brief Retrieves the blend mode used for a shader.
		 *
		 * @param shaderID The shader to look up.
		 * @return The blend mode resulting from setShaderBlendMode(), or
		 * BlendMode::Opaque when no override was registered.
		 */
		[[nodiscard]] BlendMode getShaderBlendMode(uint32_t shaderID) const;

		/**
		 * @brief Sets the ordering strategy used for opaque draws.
		 *
		 * @param mode The ordering strategy to use.
		 */
		void setOpaqueSortMode(OpaqueSortMode mode)
		{
			_opaqueSortMode = mode;
		}

		/**
		 * @brief Retrieves the ordering strategy used for opaque draws.
		 *
		 * @return The current ordering strategy.
		 */
		[[nodiscard]] OpaqueSortMode getOpaqueSortMode() const
		{
			return _opaqueSortMode;
		}

		protected:
		std::map<PipelineVariant, VkPipeline>
			_pipelines;	   ///< Graphics pipeline per (shader, blend mode)
						   ///< variant. Each shader has an opaque and an
						   ///< alpha-blended pipeline so that blending and
						   ///< depth writes are only enabled for the draws that
						   ///< need them.

		std::map<uint32_t, VkPipelineLayout>
			_pipelineLayouts;	 ///< A map of pipeline layer identifiers to
								 ///< Vulkan pipeline layout objects. This map
								 ///< is used to manage different pipeline
								 ///< layouts for rendering operations based on
								 ///< the pipeline layer associated with render
								 ///< objects. Each entry in the map corresponds
								 ///< to a specific pipeline layer and its
								 ///< associated Vulkan pipeline layout, which
								 ///< defines the interface between shader
								 ///< stages and the resources bound to the
								 ///< graphics pipeline for rendering objects
								 ///< that belong to that layer.

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
		std::vector<std::shared_ptr<Frame>> _frames;

		/**
		 * @brief The device context used to create this renderer's resources.
		 *
		 * Kept alive for the lifetime of the Renderer so that frames created
		 * from it can safely destroy their Vulkan resources on destruction.
		 */
		std::shared_ptr<DeviceContext> _deviceContext;

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
		 * @brief Resolves the graphics pipeline bound to a shader ID.
		 *
		 * Performs a single bounds-checked index into _pipelines and returns
		 * VK_NULL_HANDLE when the shader ID is out of range, has no pipeline,
		 * or is the invalid ID 0 returned by
		 * RessourceProvider::getShaderID().
		 *
		 * @param shaderID Shader ID to resolve.
		 *
		 * @return The matching pipeline, or VK_NULL_HANDLE if there is none.
		 */
		[[nodiscard]] VkPipeline pipelineFor(uint32_t shaderID) const
		{
			return shaderID < _pipelines.size()
				? _pipelines[shaderID]
				: VK_NULL_HANDLE;
		}

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
								 const utility::graphic::ViewF &view,
								 std::size_t viewSlot);

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
		 *
		 * @param view The view state used to build the culling frustum.
		 */
		void recordCommandBuffer(VkRenderPass renderPass,
								 VkFramebuffer swapChainFramebuffer,
								 VkExtent2D swapChainExtent, const Scene &scene,
								 std::size_t viewSlot,
								 const utility::graphic::ViewF &view);

		/**
		 * @brief Finds the graphics pipeline of a (shader, blend mode) pair.
		 *
		 * @param shaderID The shader identifying the pipeline family.
		 * @param blendMode The blend mode variant to select.
		 * @return The matching pipeline, or VK_NULL_HANDLE when no pipeline was
		 * created for that pair.
		 */
		VkPipeline pipelineFor(uint32_t shaderID, BlendMode blendMode) const;

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
		void createGraphicsPipelines(VkDevice device, VkRenderPass renderPass,
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

		std::shared_ptr<RessourceManager>
			_ressourceManager;	  ///< A shared pointer to the RessourceManager,
								  ///< which is responsible for managing
								  ///< materials, textures, and other resources
								  ///< used in rendering. The Renderer interacts
								  ///< with the RessourceManager to access the
								  ///< necessary resources for rendering scenes,
								  ///< such as materials and textures associated
								  ///< with render objects. This member variable
								  ///< allows the Renderer to efficiently manage
								  ///< and utilize resources during the
								  ///< rendering process.

		/**
		 * @brief Whether frustum and distance culling are active.
		 */
		bool _cullingEnabled = true;

		/**
		 * @brief Maximum draw distance for distance culling (0 disables it).
		 */
		float _maxDrawDistance = 0.0f;

		/**
		 * @brief Whether identical meshes are merged into instanced draws.
		 */
		bool _instancingEnabled = false;

		/**
		 * @brief Whether draws are batched through an indirect draw buffer.
		 */
		bool _indirectDrawingEnabled = false;

		/**
		 * @brief Per-shader blend mode overrides declared by the application.
		 *
		 * A shader missing from the map uses the blend mode derived from the
		 * material of each mesh.
		 */
		std::map<uint32_t, BlendMode> _shaderBlendModes;

		/**
		 * @brief Ordering strategy applied to the opaque draws of a frame.
		 */
		OpaqueSortMode _opaqueSortMode = OpaqueSortMode::FrontToBack;
	};
}	 // namespace evan
