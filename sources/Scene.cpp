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
