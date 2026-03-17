/*
** ETIB PROJECT, 2026
** evan
** File description:
** Scene
*/

#include "Scene.hpp"

#include "Renderer.hpp"

evan::Scene::Scene(const DeviceContext &deviceContext, const Renderer &renderer,
                std::vector<std::string> texturePaths, std::map<std::string, std::vector<Mesh>> meshData)
{
    // Create materials for each texture path
    for (const auto &texturePath : texturePaths) {
        _materials.emplace(_materials.size(), Material(deviceContext, renderer, texturePath));

        auto correspondingMeshData = meshData.find(texturePath);

        if (correspondingMeshData != meshData.end()) {
            // If there is a mesh data corresponding to the texture path, create a GPUMesh for each mesh
            for (const auto &mesh : correspondingMeshData->second) {
                _meshes.emplace_back(deviceContext, mesh.vertices, mesh.indices, _materials.size() - 1);
            }
        }
    }
}

evan::Scene::~Scene()
{
}

void evan::Scene::destroy(VkDevice device)
{
    for (auto &mesh : _meshes) {
        mesh.destroy(device);
    }

    for (auto &[_, material] : _materials) {
        material.destroy(device);
    }
}

/////////////
// Getters //
/////////////

std::vector<evan::GPUMesh> evan::Scene::getMeshes() const
{
    return _meshes;
}

std::map<uint32_t, evan::Material> evan::Scene::getMaterials() const
{
    return _materials;
}

VkBuffer * evan::Scene::getVertexBuffers() const
{
    VkBuffer *vertexBuffers = new VkBuffer[_meshes.size()];

    for (size_t i = 0; i < _meshes.size(); i++) {
        vertexBuffers[i] = _meshes[i].getVertexBuffer();
    }
    return vertexBuffers;
}

VkBuffer * evan::Scene::getIndexBuffers() const
{
    VkBuffer *indexBuffers = new VkBuffer[_meshes.size()];

    for (size_t i = 0; i < _meshes.size(); i++) {
        indexBuffers[i] = _meshes[i].getIndexBuffer();
    }
    return indexBuffers;
}

