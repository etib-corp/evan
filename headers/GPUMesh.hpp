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

namespace evan {
    class GPUMesh {
        public:
        GPUMesh(const DeviceContext &deviceContext, std::vector<Vertex> vertices, std::vector<uint32_t> indices, uint32_t materialID);
        ~GPUMesh();

        void destroy(VkDevice device);

        VkBuffer getVertexBuffer() const;
        VkBuffer getIndexBuffer() const;
        uint32_t getIndexCount() const;
        uint32_t getMaterialID() const;

        protected:
        VkBuffer _vertexBuffer;
        VkDeviceMemory _vertexBufferMemory;
        VkBuffer _indexBuffer;
        VkDeviceMemory _indexBufferMemory;
        uint32_t _indexCount;
        uint32_t _materialID;

        void createIndexBuffer(const DeviceContext &deviceContext, std::vector<uint32_t> indices);
    };
}
