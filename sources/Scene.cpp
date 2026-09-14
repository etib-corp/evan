/*
** ETIB PROJECT, 2026
** evan
** File description:
** Scene
*/

#include "evan/Scene.hpp"

#include "evan/Renderer.hpp"

evan::Scene::~Scene()
{
	this->getLogger().info() << "Destroying Scene...";
}

////////////////////
// Public Methods //
////////////////////

void evan::Scene::destroy(VkDevice device)
{
	this->getLogger().info()
		<< "Destroying Scene and cleaning up GPU resources...";

	this->getLogger().info() << "Destroying renderable objects...";
	for (auto &[id, object]: _objects) {
		this->getLogger().info()
			<< "Destroying renderable object with ID " << id;
		object->destroy(device);
	}

	for (auto &[id, material]: _materials) {
		this->getLogger().info() << "Destroying material with ID " << id;
		material->destroy(device);
	}
}

size_t evan::Scene::addObject(uint32_t objectID,
							  std::shared_ptr<RenderObject> renderObject)
{
	this->getLogger().info()
		<< "Adding renderable object with ID " << objectID << " to Scene...";
	_objects[objectID] = renderObject;
	_meshesCacheDirty = true;
	return objectID;
}

bool evan::Scene::removeObject(uint32_t objectID)
{
	this->getLogger().info() << "Removing renderable object with ID "
							 << objectID << " from Scene...";
	bool removed = _objects.erase(objectID) > 0;
	if (removed) {
		_meshesCacheDirty = true;
	}
	return removed;
}

std::shared_ptr<evan::RenderObject>
	evan::Scene::getObject(uint32_t objectID) const
{
	auto objectIt = _objects.find(objectID);
	if (objectIt == _objects.end()) {
		return nullptr;
	}
	return objectIt->second;
}

/////////////
// Getters //
/////////////

const std::vector<std::shared_ptr<evan::GPUMesh>> &
	evan::Scene::getMeshes() const
{
	if (_meshesCacheDirty) {
		_meshesCache.clear();

		size_t meshCount = 0;
		for (const auto &[_, object]: _objects) {
			if (object) {
				meshCount += object->getMeshes().size();
			}
		}
		_meshesCache.reserve(meshCount);

		for (const auto &[_, object]: _objects) {
			if (object) {
				const std::vector<std::shared_ptr<GPUMesh>> &objectMeshes =
					object->getMeshes();
				_meshesCache.insert(_meshesCache.end(), objectMeshes.begin(),
									objectMeshes.end());
			}
		}
		_meshesCacheDirty = false;
	}
	return _meshesCache;
}

const std::map<uint32_t, std::shared_ptr<evan::GPUMaterial>> &
	evan::Scene::getMaterials() const
{
	return _materials;
}
