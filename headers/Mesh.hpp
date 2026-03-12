/*
** ETIB PROJECT, 2026
** evan
** File description:
** Mesh
*/

#pragma once

#include "EvanPlatform.hpp"

#include "DeviceContext.hpp"

#include "Vertex.hpp"

namespace evan {
    class Mesh {
        public:
        Mesh(const DeviceContext &deviceContext, std::vector<Vertex> vertices, std::vector<uint32_t> indices, uint32_t materialID);
        ~Mesh();

        void createIndexBuffer(const DeviceContext &deviceContext, std::vector<uint32_t> indices);

        protected:
        VkBuffer _vertexBuffer;
        VkDeviceMemory _vertexBufferMemory;
        VkBuffer _indexBuffer;
        VkDeviceMemory _indexBufferMemory;
        uint32_t _indexCount;
        uint32_t _materialID;

    };
}
