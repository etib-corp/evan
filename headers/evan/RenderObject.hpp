/*
** ETIB PROJECT, 2026
** xider
** File description:
** RenderObject
*/

#pragma once

#include <utility/graphic/mesh.hpp>

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include <map>

#include "DeviceContext.hpp"
#include "GPUMesh.hpp"

namespace evan
{
	/**
	 * @brief A Renderable object. It is a simple interface between our
	 * core renderer and on-top built objects
	 * (such as texts, basic shapes and 3d objects)
	 */
	class RenderObject:
		protected utility::logging::Loggable<RenderObject,
											 utility::logging::DefaultLogger>
	{
		public:
		/**
		 * @brief Constructor: creates a single renderable object using
		 * its meshes and its associated material ID.
		 *
		 * @param deviceContext A shared pointer to the DeviceContext, which
		 * provides access to Vulkan device resources and operations needed for
		 * creating GPU resources for the render object.
		 * @param rawObjects A map of mesh IDs to utility::graphic::Mesh
		 * objects, representing the raw mesh data that will be used to create
		 * the GPU meshes for this render object. Each entry in the map
		 * corresponds to a mesh that will be rendered as part of this object,
		 * and the mesh data will be processed and uploaded to the GPU to create
		 * the corresponding GPUMesh instances.
		 * @param pipelineLayer A string representing the pipeline layer
		 * associated with this render object, which can be used to determine
		 * which shader program and rendering configurations should be applied
		 * when rendering this object. The pipeline layer can be used to group
		 * render objects with similar rendering requirements together, allowing
		 * for efficient batching and state management in the rendering process.
		 */
		RenderObject(
			std::shared_ptr<DeviceContext> deviceContext,
			const std::map<uint32_t, utility::graphic::Mesh> &rawObjects,
			const std::string &pipelineLayer);

		/**
		 * @brief Destructor for the RenderObject.
		 */
		~RenderObject();

		/**
		 * @brief Destroys the GPU resources associated with this RenderObject,
		 * such as the GPUMesh instances. This method should be called when the
		 * render object is no longer needed to free up GPU resources and
		 * prevent memory leaks.
		 *
		 * @param device The Vulkan device used for resource management, which
		 * is required to clean up GPU resources associated with the GPUMesh
		 * instances. It is assumed that the Vulkan device is properly
		 * initialized and can be used to manage GPU resources for cleanup.
		 *
		 * @note The method assumes that the GPUMesh instances were created
		 * using the same device context and are associated with the provided
		 * device for cleanup. It is important to ensure that all GPU resources
		 * are properly released to maintain efficient resource usage and
		 * prevent memory leaks in the rendering process.
		 */
		void destroy(VkDevice device);

		/**
		 * @brief Retrieves the vector of GPUMesh instances associated with this
		 * RenderObject. Each GPUMesh represents a mesh that will be rendered as
		 * part of this object, and contains the necessary GPU resources for
		 * rendering.
		 *
		 * @return A constant reference to a vector of GPUMesh instances
		 * representing the meshes associated with this RenderObject. These
		 * GPUMesh instances can be used for rendering operations and contain
		 * the necessary GPU resources for rendering the meshes.
		 */
		[[nodiscard]] const std::vector<std::shared_ptr<GPUMesh>> &
			getMeshes() const;

		/**
		 * @brief Retrieves the pipeline layer associated with this
		 * RenderObject. The pipeline layer is a string that can be used to
		 * determine which shader program and rendering configurations should be
		 * applied when rendering this object. It can be used to group render
		 * objects with similar rendering requirements together for efficient
		 * batching and state management in the rendering process.
		 *
		 * @return A string representing the pipeline layer associated with this
		 * RenderObject, which can be used to determine rendering configurations
		 * for this object during the rendering process.
		 */
		[[nodiscard]] std::string getPipelineLayer() const;

		protected:
		std::vector<std::shared_ptr<GPUMesh>>
			_meshes;	///< A vector of GPUMesh instances representing the
						///< meshes associated with this RenderObject. Each
						///< GPUMesh contains the necessary GPU resources for
						///< rendering a mesh that is part of this object.
		std::string
			_pipelineLayer;	   ///< A string representing the pipeline layer
							   ///< associated with this RenderObject, which can
							   ///< be used to determine rendering
							   ///< configurations for this object during the
							   ///< rendering process. The pipeline layer can be
							   ///< used to group render objects with similar
							   ///< rendering requirements together for
							   ///< efficient batching and state management in
							   ///< the rendering process.
	};
}	 // namespace evan
