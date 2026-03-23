/*
** ETIB PROJECT, 2026
** evan
** File description:
** GPUMesh
*/

#pragma once

#include "EvanPlatform.hpp"

#include "DeviceContext.hpp"

#include "Vertex.hpp"

namespace evan
{
	/**
	 * @brief A class representing a mesh stored on the GPU, containing vertex
	 * and index buffers, as well as material information.
	 *
	 * The GPUMesh class encapsulates the creation and management of vertex and
	 * index buffers on the GPU, allowing for efficient rendering of 3D models.
	 * It provides methods to retrieve the vertex and index buffers, as well as
	 * the count of indices and the material ID associated with the mesh. The
	 * class also includes a destructor to clean up GPU resources when the mesh
	 * is no longer needed.
	 *
	 * @note The GPUMesh class relies on the DeviceContext for Vulkan device
	 * management and assumes that the vertex data is provided in a specific
	 * format defined by the Vertex structure. It also assumes that the material
	 * ID is used to reference material properties for rendering.
	 */
	class GPUMesh
	{
		public:
		/**
		 * @brief Constructs a GPUMesh instance by creating vertex and index
		 * buffers on the GPU using the provided device context, vertex data,
		 * index data, and material ID.
		 *
		 * This constructor initializes the GPUMesh by creating vertex and index
		 * buffers on the GPU using the provided device context. It takes in a
		 * vector of Vertex structures representing the vertex data, a vector of
		 * uint32_t representing the index data, and a uint32_t representing the
		 * material ID associated with the mesh. The constructor is responsible
		 * for allocating GPU resources and uploading the vertex and index data
		 * to the GPU for efficient rendering.
		 *
		 * @param deviceContext A reference to the DeviceContext instance used
		 * for Vulkan device management and resource creation.
		 * @param vertices A vector of Vertex structures containing the vertex
		 * data for the mesh.
		 * @param indices A vector of uint32_t containing the index data for the
		 * mesh.
		 * @param materialID A uint32_t representing the material ID associated
		 * with the mesh, which can be used to reference material properties for
		 * rendering.
		 *
		 * @note The constructor assumes that the vertex data is provided in a
		 * specific format defined by the Vertex structure and that the material
		 * ID is used for rendering purposes. It also assumes that the
		 * DeviceContext is properly initialized and can be used to create GPU
		 * resources.
		 */
		GPUMesh(const DeviceContext &deviceContext,
				std::vector<Vertex> vertices, std::vector<uint32_t> indices,
				uint32_t materialID);

		~GPUMesh();

		/**
		 * @brief Destroys the GPUMesh instance by cleaning up GPU resources
		 * associated with the vertex and index buffers.
		 *
		 * This method is responsible for releasing GPU resources allocated for
		 * the vertex and index buffers when the GPUMesh instance is destroyed.
		 * It takes in a VkDevice representing the Vulkan device used for
		 * resource management and ensures that all GPU resources are properly
		 * cleaned up to prevent memory leaks and ensure efficient resource
		 * usage.
		 *
		 * @param device The Vulkan device used for resource management, which
		 * is required to clean up GPU resources associated with the vertex and
		 * index buffers.
		 *
		 * @note The method assumes that the Vulkan device is properly
		 * initialized and can be used to manage GPU resources. It also assumes
		 * that the vertex and index buffers were created using the same device
		 * context and are associated with the provided device for cleanup.
		 */
		void destroy(VkDevice device);

		/**
		 * @brief Retrieves the Vulkan vertex buffer associated with this
		 * GPUMesh instance.
		 *
		 * This method returns the Vulkan buffer handle for the vertex buffer,
		 * which can be used for rendering operations. The vertex buffer
		 * contains the vertex data for the mesh and is created during the
		 * initialization of the GPUMesh instance.
		 *
		 * @return VkBuffer The Vulkan buffer handle for the vertex buffer
		 * associated with this GPUMesh instance.
		 */
		VkBuffer getVertexBuffer() const;

		/**
		 * @brief Retrieves the Vulkan index buffer associated with this GPUMesh
		 * instance.
		 *
		 * This method returns the Vulkan buffer handle for the index buffer,
		 * which can be used for rendering operations. The index buffer contains
		 * the index data for the mesh and is created during the initialization
		 * of the GPUMesh instance.
		 *
		 * @return VkBuffer The Vulkan buffer handle for the index buffer
		 * associated with this GPUMesh instance.
		 */
		VkBuffer getIndexBuffer() const;

		/**
		 * @brief Retrieves the count of indices in the index buffer associated
		 * with this GPUMesh instance.
		 *
		 * This method returns the number of indices stored in the index buffer,
		 * which is used for rendering operations to determine how many vertices
		 * to draw based on the index data.
		 *
		 * @return uint32_t The count of indices in the index buffer associated
		 * with this GPUMesh instance.
		 */
		uint32_t getIndexCount() const;

		/**
		 * @brief Retrieves the material ID associated with this GPUMesh
		 * instance.
		 *
		 * This method returns the material ID, which can be used to reference
		 * material properties for rendering. The material ID is set during the
		 * initialization of the GPUMesh instance and can be used to determine
		 * how the mesh should be rendered based on its associated material
		 * properties.
		 *
		 * @return uint32_t The material ID associated with this GPUMesh
		 * instance, which can be used for rendering purposes to reference
		 * material properties.
		 */
		uint32_t getMaterialID() const;

		protected:
		/**
		 * @brief Creates the vertex buffer on the GPU using the provided device
		 * context and vertex data.
		 *
		 * This method is responsible for creating the Vulkan vertex buffer and
		 * allocating memory for it on the GPU. It takes in a reference to the
		 * DeviceContext for Vulkan device management and a vector of Vertex
		 * structures containing the vertex data to be uploaded to the GPU. The
		 * method handles the creation of the vertex buffer, allocation of
		 * memory, and uploading of vertex data to ensure that the vertex buffer
		 * is properly set up for rendering operations.
		 *
		 * @param deviceContext A reference to the DeviceContext instance used
		 * for Vulkan device management and resource creation.
		 * @param vertices A vector of Vertex structures containing the vertex
		 * data to be uploaded to the GPU for the vertex buffer.
		 *
		 * @note The method assumes that the DeviceContext is properly
		 * initialized and can be used to create GPU resources. It also assumes
		 * that the vertex data is provided in a specific format defined by the
		 * Vertex structure and that the Vulkan device associated with the
		 * DeviceContext can be used for resource management.
		 */
		void createIndexBuffer(const DeviceContext &deviceContext,
							   std::vector<uint32_t> indices);

		/**
		 * Vulkan buffer handle for the vertex buffer
		 * associated with this GPUMesh instance,
		 * which contains the vertex data for rendering operations.
		 */
		VkBuffer _vertexBuffer;

		/**
		 * Vulkan device memory handle for the vertex buffer
		 * associated with this GPUMesh instance,
		 * which is used to manage the memory allocated for the vertex buffer on
		 * the GPU.
		 */
		VkDeviceMemory _vertexBufferMemory;

		/**
		 * Vulkan buffer handle for the index buffer
		 * associated with this GPUMesh instance,
		 * which contains the index data for rendering operations.
		 */
		VkBuffer _indexBuffer;

		/**
		 * Vulkan device memory handle for the index buffer
		 * associated with this GPUMesh instance,
		 * which is used to manage the memory allocated for the index buffer on
		 * the GPU.
		 */
		VkDeviceMemory _indexBufferMemory;

		/**
		 * The count of indices stored in the index buffer
		 * associated with this GPUMesh instance,
		 * which is used for rendering operations to determine how many vertices
		 * to draw based on the index data.
		 */
		uint32_t _indexCount;

		/**
		 * The material ID associated with this GPUMesh instance,
		 * which can be used for rendering purposes to reference material
		 * properties.
		 */
		uint32_t _materialID;
	};
}	 // namespace evan
