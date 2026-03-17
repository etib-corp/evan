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

namespace evan {
    class Renderer;

    class Scene {
        public:
        Scene(const DeviceContext &deviceContext, const Renderer &renderer,
                std::vector<std::string> texturePaths, std::map<std::string, std::vector<Mesh>> meshData);
        ~Scene();

        void destroy(VkDevice device);

        const std::vector<GPUMesh> &getMeshes() const;
        const std::map<uint32_t, Material> &getMaterials() const;

        VkBuffer * getVertexBuffers() const;
        VkBuffer * getIndexBuffers() const;

        protected:
        std::vector<GPUMesh> _meshes;
        std::map<uint32_t, Material> _materials;

        private:
    };
}
