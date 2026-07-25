/*
** ETIB PROJECT, 2026
** evan
** File description:
** Scene
*/

#pragma once

#include <utility/graphic/material.hpp>

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include "evan/EvanPlatform.hpp"

#include "evan/GPUMesh.hpp"
#include "evan/GPUMaterial.hpp"

#include "RenderObject.hpp"

#include <map>

namespace evan
{
	class Renderer;

	/**
	 * @brief The Scene class represents a collection of meshes and materials
	 * that can be rendered.
	 *
	 * It manages the GPU resources associated with these meshes and materials,
	 * such as vertex buffers and index buffers. The Scene class provides
	 * methods to access the meshes and materials, as well as to destroy the GPU
	 * resources when they are no longer needed.
	 *
	 * @note The Scene class is designed to work with the Vulkan API, and it
	 * assumes that the necessary Vulkan resources (such as the device and
	 * renderer) are provided during construction.
	 */
	class Scene:
		protected utility::logging::Loggable<Scene,
											 utility::logging::DefaultLogger>
	{
		public:
		Scene() = default;

		~Scene();

		/**
		 * @brief Destroys the Scene by cleaning up GPU resources associated
		 * with the meshes and materials.
		 *
		 * This method is responsible for releasing GPU resources allocated for
		 * the vertex and index buffers of the meshes, as well as any resources
		 * associated with the materials. It takes a VkDevice representing the
		 * Vulkan device used for resource management and ensures that all GPU
		 * resources are properly cleaned up to prevent memory leaks and ensure
		 * efficient resource usage.
		 *
		 * @param device The Vulkan device used for resource management, which
		 * is required to clean up GPU resources associated with the meshes and
		 * materials in the scene.
		 *
		 * @note The method assumes that the Vulkan device is properly
		 * initialized and can be used to manage GPU resources. It also assumes
		 * that the meshes and materials were created using the same device
		 * context and are associated with the provided device for cleanup.
		 */
		void destroy(VkDevice device);

		/**
		 * @brief Adds a renderable object to the Scene with the specified
		 * object ID.
		 *
		 * This method allows adding a renderable object, represented by a
		 * RenderObject instance, to the Scene. The object is associated with a
		 * unique object ID, which can be used to manage and access the
		 * renderable objects in the Scene. The RenderObject contains the
		 * necessary information about the meshes and materials for rendering
		 * the object as part of the Scene.
		 *
		 * @param objectID A uint32_t representing the unique identifier for the
		 * renderable object being added to the Scene. This ID can be used to
		 * manage and access the object within the Scene.
		 * @param renderObject A shared pointer to a RenderObject instance
		 * representing the renderable object being added to the Scene. The
		 * RenderObject contains the necessary information about the meshes and
		 * materials for rendering the object as part of the Scene. The shared
		 * pointer allows for efficient memory management and ensures that the
		 * RenderObject is properly cleaned up when it is no longer needed.
		 * @return A size_t representing the object ID of the added renderable
		 * object. This ID can be used to manage and access the object within
		 * the Scene.
		 */
		size_t addObject(uint32_t objectID,
						 std::shared_ptr<RenderObject> renderObject);

		/**
		 * @brief Removes a renderable object from the Scene based on the
		 * specified object ID.
		 *
		 * This method allows removing a renderable object from the Scene using
		 * its unique object ID. The method checks if the object with the
		 * specified ID exists in the Scene and, if it does, removes it from the
		 * internal map of objects. This is useful for managing the renderable
		 * objects in the Scene and ensuring that resources associated with
		 * removed objects can be cleaned up properly.
		 *
		 * @param objectID A uint32_t representing the unique identifier for the
		 * renderable object to be removed from the Scene. This ID is used to
		 * locate and remove the corresponding RenderObject from the internal
		 * map of objects in the Scene.
		 *
		 * @return A boolean value indicating whether the object was
		 * successfully removed from the Scene. The method returns true if the
		 * object was found and removed, and false if the object with the
		 * specified ID was not found.
		 */
		bool removeObject(uint32_t objectID);

		/**
		 * @brief Retrieves the meshes contained in the Scene.
		 *
		 * This method returns a constant reference to a vector of GPUMesh
		 * objects representing the meshes contained in the Scene. Each GPUMesh
		 * object contains information about the vertex and index buffers, as
		 * well as the material ID associated with that mesh. The returned
		 * vector allows access to the meshes for rendering or other operations
		 * while ensuring that the internal state of the Scene is not modified.
		 *
		 * @return A constant reference to a vector of GPUMesh objects
		 * representing the meshes contained in the Scene.
		 *
		 * @note The method assumes that the meshes have been properly
		 * initialized and are ready for use in rendering or other operations.
		 * It also assumes that the caller will not modify the returned vector,
		 * as it is a constant reference to the internal state of the Scene.
		 */
		const std::vector<std::shared_ptr<GPUMesh>> &getMeshes() const;

		/**
		 * @brief Retrieves the materials contained in the Scene.
		 *
		 * This method returns a constant reference to a map where the key is a
		 * uint32_t representing the material ID, and the value is a GPUMaterial
		 * object representing the material properties associated with that
		 * material ID. The returned map allows access to the materials for
		 * rendering or other operations while ensuring that the internal state
		 * of the Scene is not modified.
		 *
		 * @return A constant reference to a map of material IDs to GPUMaterial
		 * objects representing the materials contained in the Scene.
		 *
		 * @note The method assumes that the materials have been properly
		 * initialized and are ready for use in rendering or other operations.
		 * It also assumes that the caller will not modify the returned map, as
		 * it is a constant reference to the internal state of the Scene.
		 */
		const std::map<uint32_t, std::shared_ptr<evan::GPUMaterial>> &
			getMaterials() const;

		protected:
		/**
		 * @brief A map of object IDs to RenderObject objects representing the
		 * objects contained in the Scene.
		 *
		 * The key of the map is a uint32_t representing the object ID, and the
		 * value is a RenderObject object representing the renderable object
		 * associated with that object ID. This map is used to store and manage
		 * the renderable objects in the Scene for rendering and other
		 * operations.
		 */
		std::map<uint32_t, std::shared_ptr<RenderObject>> _objects;

		/**
		 * @brief A map of material IDs to GPUMaterial objects representing the
		 * materials contained in the Scene.
		 *
		 * The key of the map is a uint32_t representing the material ID, and
		 * the value is a GPUMaterial object representing the material
		 * properties associated with that material ID. This map is used to
		 * store and manage the materials in the Scene for rendering and other
		 * operations.
		 *
		 * @note The map is initialized during the construction of the Scene and
		 * is used to access the materials for rendering or other operations. It
		 * should be properly managed to ensure that GPU resources are cleaned
		 * up when the Scene is destroyed.
		 */
		std::map<uint32_t, std::shared_ptr<GPUMaterial>> _materials;
	};
}	 // namespace evan
