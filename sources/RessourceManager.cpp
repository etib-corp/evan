/*
** ETIB PROJECT, 2026
** xider
** File description:
** RessourceManager
*/

#include "evan/RessourceManager.hpp"

evan::RessourceManager::RessourceManager(
	std::shared_ptr<utility::RessourceProvider> ressourceProvider,
	std::shared_ptr<DeviceContext> deviceContext)
	: _ressourceProvider(ressourceProvider)
	, _deviceContext(deviceContext)
{
	this->getLogger().info() << "Initializing RessourceManager...";

	const auto &shaders = _ressourceProvider->getShaders();

	this->getLogger().info() << "Loading shaders from RessourceProvider...";
	for (const auto &[id, shader]: shaders) {
		this->getLogger().info() << "Loading shader ID " << id;
		_shaders[id] = std::make_shared<GPUShader>(
			_deviceContext->getDeviceBackend()->getDevice(), *shader);
	}
}

evan::RessourceManager::~RessourceManager()
{
	this->getLogger().info()
		<< "Destroying RessourceManager and cleaning up GPU resources...";

	if (!_deviceContext) {
		return;
	}

	VkDevice device = _deviceContext->getDeviceBackend()->getDevice();

	for (auto &[id, material]: _materials) {
		material->destroy(device);
	}
	_materials.clear();

	for (auto &[id, texture]: _textures) {
		texture->destroy(device);
	}
	_textures.clear();

	for (auto &[id, shader]: _shaders) {
		shader->destroy();
	}
	_shaders.clear();
}

////////////////////
// Public Methods //
////////////////////

void evan::RessourceManager::cleanup()
{
	this->getLogger().info() << "Releasing cached GPU resources...";
	_materials.clear();
	_textures.clear();
	_shaders.clear();
}

void evan::RessourceManager::init(std::shared_ptr<Renderer> renderer)
{
	this->getLogger().info()
		<< "Initializing RessourceManager with Renderer...";

	_renderer = renderer;
	sync();
}

void evan::RessourceManager::sync(bool refresh)
{
	this->getLogger().debug() << "Synchronizing RessourceManager...";
	auto renderer = _renderer.lock();
	if (!renderer) {
		this->getLogger().info()
			<< "Renderer not yet initialized. Skipping synchronization.";
		return;
	}

	const uint64_t version = _ressourceProvider->version();

	if (!refresh && _synchronized && version == _syncedVersion) {
		return;
	}

	VkDevice device = _deviceContext->getDeviceBackend()->getDevice();

	const auto &shaders   = _ressourceProvider->getShaders();
	const auto &materials = _ressourceProvider->getMaterials();
	const auto &textures  = _ressourceProvider->getTextures();

	this->getLogger().debug() << "Synchronizing shaders...";
	for (const auto &[id, shader]: shaders) {
		this->getLogger().debug() << "Synchronizing shader ID " << id;
		auto it = _shaders.find(id);

		if (it == _shaders.end()) {
			this->getLogger().info()
				<< "Creating new GPUShader for shader ID " << id;
			_shaders[id] = std::make_shared<GPUShader>(
				_deviceContext->getDeviceBackend()->getDevice(), *shader);
		}
		if (refresh) {
			this->getLogger().debug()
				<< "Refreshing GPUShader for shader ID " << id;
			if (auto it = _shaders.find(id); it != _shaders.end()) {
				it->second->destroy();
			}
			_shaders[id] = std::make_shared<GPUShader>(
				_deviceContext->getDeviceBackend()->getDevice(), *shader);
		}
	}

	this->getLogger().debug() << "Synchronizing materials...";
	for (const auto &[id, material]: materials) {
		this->getLogger().debug() << "Synchronizing material ID " << id;
		auto shaderID =
			_ressourceProvider->getShaderID(material->getShaderName());
		auto it = _materials.find(id);

		if (it == _materials.end()) {
			if (shaderID == 0) {
				this->getLogger().warning()
					<< "Shader '" << material->getShaderName()
					<< "' not found for material ID " << id;
				continue;	 // Skip this material if its shader is not found
			}
			this->getLogger().info()
				<< "Creating new GPUMaterial for material ID " << id;
			_materials[id] = std::make_shared<GPUMaterial>(
				_deviceContext, *renderer, *material, shaderID);
		} else if (refresh) {
			if (shaderID == 0) {
				this->getLogger().warning()
					<< "Shader '" << material->getShaderName()
					<< "' not found for material ID " << id;
				continue;	 // Skip this material if its shader is not found
			}
			this->getLogger().debug()
				<< "Refreshing GPUMaterial for material ID " << id;
			it->second->destroy(device);
			_materials[id] = std::make_shared<GPUMaterial>(
				_deviceContext, *renderer, *material, shaderID);
		} else {
			_materials[id]->update(_deviceContext, *renderer, *material,
								   shaderID);
		}
	}

	this->getLogger().debug() << "Synchronizing textures...";
	for (const auto &[id, texture]: textures) {
		this->getLogger().debug() << "Synchronizing texture ID " << id;
		auto it = _textures.find(id);

		if (it == _textures.end()) {
			// It only creates a Albedo texture for now,
			// but it will be extended in the future to support
			// other types of textures (normal, roughness, etc.) based on the
			// material properties.
			_textures[id] =
				std::make_shared<GPUTexture>(_deviceContext, *texture);
		} else if (refresh) {
			this->getLogger().debug()
				<< "Refreshing GPUTexture for texture ID " << id;
			it->second->destroy(device);
			_textures[id] =
				std::make_shared<GPUTexture>(_deviceContext, *texture);
		}
	}

	_syncedVersion = version;
	_synchronized  = true;
}

/////////////
// Getters //
/////////////

std::shared_ptr<evan::GPUMaterial>
	evan::RessourceManager::getMaterial(uint32_t id) const
{
	auto it = _materials.find(id);

	if (it != _materials.end()) {
		return it->second;
	}
	return nullptr;
}

std::shared_ptr<evan::GPUTexture>
	evan::RessourceManager::getTexture(uint32_t id) const
{
	auto it = _textures.find(id);

	if (it != _textures.end()) {
		return it->second;
	}
	return nullptr;
}

std::shared_ptr<evan::GPUShader>
	evan::RessourceManager::getShader(uint32_t id) const
{
	auto it = _shaders.find(id);

	if (it != _shaders.end()) {
		return it->second;
	}
	return nullptr;
}

const std::unordered_map<uint32_t, std::shared_ptr<evan::GPUShader>> &
	evan::RessourceManager::getShaders() const
{
	return _shaders;
}
