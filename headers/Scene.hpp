/*
** ETIB PROJECT, 2026
** evan
** File description:
** Scene
*/

#pragma once

#include "EvanPlatform.hpp"

#include "Mesh.hpp"
#include "GPUMesh.hpp"
#include "Material.hpp"

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
	class Scene
	{
		public:
		/**
		 * @brief Constructs a Scene object by loading meshes and materials from
		 * the provided data.
		 *
		 * This constructor initializes the Scene by loading meshes and
		 * materials based on the provided texture paths and mesh data. It takes
		 * references to the DeviceContext and Renderer to access the necessary
		 * Vulkan resources for creating GPU meshes. The constructor processes
		 * the mesh data, creates GPUMesh instances for each mesh, and stores
		 * them in the _meshes vector. It also creates Material instances for
		 * each unique material ID found in the mesh data and stores them in the
		 * _materials map.
		 *
		 * @param deviceContext A reference to the DeviceContext, which provides
		 * access to Vulkan resources needed for creating GPU meshes.
		 * @param renderer A reference to the Renderer, which may be used for
		 * rendering the meshes and materials in the scene.
		 * @param texturePaths A vector of strings representing the file paths
		 * to the textures used in the scene. These textures may be associated
		 * with the materials used by the meshes.
		 * @param meshData A map where the key is a string representing the mesh
		 * name, and the value is a vector of Mesh objects representing the mesh
		 * data for that mesh name. Each Mesh object contains information about
		 * the vertices, indices, and material ID associated with that mesh.
		 *
		 * @note The constructor is responsible for processing the mesh data,
		 * creating GPUMesh instances for each mesh, and creating Material
		 * instances for each unique material ID found in the mesh data. It also
		 * handles any necessary Vulkan resource management for the meshes and
		 * materials.
		 */
		Scene(const DeviceContext &deviceContext, const Renderer &renderer,
			  std::vector<std::string> texturePaths,
			  std::map<std::string, std::vector<Mesh>> meshData);

		Scene() = default;

		~Scene();

		/**
		 * @brief Updates the Scene by loading new meshes and materials from the
		 * provided data.
		 *
		 * This method allows updating the Scene with new mesh and material data. It
		 * takes references to the DeviceContext and Renderer to access the necessary
		 * Vulkan resources for creating GPU meshes. The method processes the new mesh data,
		 * creates GPUMesh instances for each new mesh, and updates the _meshes vector
		 * accordingly. It also creates Material instances for any new unique material IDs found in the new mesh data
		 * and updates the _materials map accordingly.
		 */
		void updateScene(const DeviceContext &deviceContext, const Renderer &renderer,
						 std::vector<std::string> texturePaths,
						 std::map<std::string, std::vector<Mesh>> meshData);

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
		const std::vector<GPUMesh> &getMeshes() const;

		/**
		 * @brief Retrieves the materials contained in the Scene.
		 *
		 * This method returns a constant reference to a map where the key is a
		 * uint32_t representing the material ID, and the value is a Material
		 * object representing the material properties associated with that
		 * material ID. The returned map allows access to the materials for
		 * rendering or other operations while ensuring that the internal state
		 * of the Scene is not modified.
		 *
		 * @return A constant reference to a map of material IDs to Material
		 * objects representing the materials contained in the Scene.
		 *
		 * @note The method assumes that the materials have been properly
		 * initialized and are ready for use in rendering or other operations.
		 * It also assumes that the caller will not modify the returned map, as
		 * it is a constant reference to the internal state of the Scene.
		 */
		const std::map<uint32_t, Material> &getMaterials() const;

		/**
		 * @brief Retrieves the vertex buffers associated with the meshes in the
		 * Scene.
		 *
		 * This method returns a pointer to an array of VkBuffer objects
		 * representing the vertex buffers associated with the meshes in the
		 * Scene. Each VkBuffer corresponds to a vertex buffer used for
		 * rendering the meshes. The caller can use these vertex buffers for
		 * rendering operations or other GPU-related tasks.
		 *
		 * @return A pointer to an array of VkBuffer objects representing the
		 * vertex buffers associated with the meshes in the Scene.
		 *
		 * @note The method assumes that the vertex buffers have been properly
		 * initialized and are ready for use in rendering or other GPU-related
		 * operations. It also assumes that the caller will manage the lifetime
		 * of the returned pointer and ensure that it is properly cleaned up
		 * when no longer needed.
		 */
		VkBuffer *getVertexBuffers() const;

		/**
		 * @brief Retrieves the index buffers associated with the meshes in the
		 * Scene.
		 *
		 * This method returns a pointer to an array of VkBuffer objects
		 * representing the index buffers associated with the meshes in the
		 * Scene. Each VkBuffer corresponds to an index buffer used for
		 * rendering the meshes. The caller can use these index buffers for
		 * rendering operations or other GPU-related tasks.
		 *
		 * @return A pointer to an array of VkBuffer objects representing the
		 * index buffers associated with the meshes in the Scene.
		 *
		 * @note The method assumes that the index buffers have been properly
		 * initialized and are ready for use in rendering or other GPU-related
		 * operations. It also assumes that the caller will manage the lifetime
		 * of the returned pointer and ensure that it is properly cleaned up
		 * when no longer needed.
		 */
		VkBuffer *getIndexBuffers() const;

		protected:
		/**
		 * @brief A vector of GPUMesh objects representing the meshes contained
		 * in the Scene.
		 *
		 * Each GPUMesh object contains information about the vertex and index
		 * buffers, as well as the material ID associated with that mesh. This
		 * vector is used to store and manage the meshes in the Scene for
		 * rendering and other operations.
		 *
		 * @note The vector is initialized during the construction of the Scene
		 * and is used to access the meshes for rendering or other operations.
		 * It should be properly managed to ensure that GPU resources are
		 * cleaned up when the Scene is destroyed.
		 */
		std::vector<GPUMesh> _meshes;

		/**
		 * @brief A map of material IDs to Material objects representing the
		 * materials contained in the Scene.
		 *
		 * The key of the map is a uint32_t representing the material ID, and
		 * the value is a Material object representing the material properties
		 * associated with that material ID. This map is used to store and
		 * manage the materials in the Scene for rendering and other operations.
		 *
		 * @note The map is initialized during the construction of the Scene and
		 * is used to access the materials for rendering or other operations. It
		 * should be properly managed to ensure that GPU resources are cleaned
		 * up when the Scene is destroyed.
		 */
		std::map<uint32_t, Material> _materials;
	};
}	 // namespace evan
