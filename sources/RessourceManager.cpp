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

	std::map<uint32_t, std::shared_ptr<utility::graphic::Shader>> shaders =
		_ressourceProvider->getShaders();

	this->getLogger().info() << "Loading shaders from RessourceProvider...";
	for (const auto &[id, shader]: shaders) {
		this->getLogger().info() << "Loading shader ID " << id;
		_shaders[id] = std::make_shared<GPUShader>(
			_deviceContext->getDeviceBackend()->_device, *shader);
	}
}

evan::RessourceManager::~RessourceManager()
{
	this->getLogger().info()
		<< "Destroying RessourceManager and cleaning up GPU resources...";
}

////////////////////
// Public Methods //
////////////////////

void evan::RessourceManager::init(std::shared_ptr<Renderer> renderer)
{
	this->getLogger().info()
		<< "Initializing RessourceManager with Renderer...";

	_renderer = renderer;
	sync();
}

void evan::RessourceManager::sync(bool refresh)
{
	this->getLogger().info() << "Synchronizing RessourceManager...";
	std::map<uint32_t, std::shared_ptr<utility::graphic::Shader>> shaders =
		_ressourceProvider->getShaders();
	std::map<uint32_t, std::shared_ptr<utility::graphic::Material>> materials =
		_ressourceProvider->getMaterials();
	std::map<uint32_t, std::shared_ptr<utility::graphic::Texture>> textures =
		_ressourceProvider->getTextures();

	this->getLogger().info() << "Synchronizing shaders...";
	for (const auto &[id, shader]: shaders) {
		this->getLogger().info() << "Synchronizing shader ID " << id;
		if (_shaders.find(id) == _shaders.end()) {
			this->getLogger().info()
				<< "Creating new GPUShader for shader ID " << id;
			_shaders[id] = std::make_shared<GPUShader>(
				_deviceContext->getDeviceBackend()->_device, *shader);
		}
		if (refresh) {
			this->getLogger().info()
				<< "Refreshing GPUShader for shader ID " << id;
			_shaders[id]->destroy();
			_shaders[id] = std::make_shared<GPUShader>(
				_deviceContext->getDeviceBackend()->_device, *shader);
		}
	}

	this->getLogger().info() << "Synchronizing materials...";
	for (const auto &[id, material]: materials) {
		this->getLogger().info() << "Synchronizing material ID " << id;
		auto shaderID =
			_ressourceProvider->getShaderID(material->getShaderName());

		if (_materials.find(id) == _materials.end()) {
			this->getLogger().info()
				<< "Creating new GPUMaterial for material ID " << id;

			if (shaderID == 0) {
				this->getLogger().warning()
					<< "Shader '" << material->getShaderName()
					<< "' not found for material ID " << id;
				continue;	 // Skip this material if its shader is not found
			}
			_materials[id] = std::make_shared<GPUMaterial>(
				_deviceContext, *_renderer, *material, shaderID);
		}
		if (refresh) {
			this->getLogger().info()
				<< "Refreshing GPUMaterial for material ID " << id;
			if (shaderID == 0) {
				this->getLogger().warning()
					<< "Shader '" << material->getShaderName()
					<< "' not found for material ID " << id;
				continue;	 // Skip this material if its shader is not found
			}
			_materials[id] = std::make_shared<GPUMaterial>(
				_deviceContext, *_renderer, *material, shaderID);
		}
		_materials[id]->update(_deviceContext, *_renderer, *material, shaderID);
	}

	this->getLogger().info() << "Synchronizing textures...";
	for (const auto &[id, texture]: textures) {
		this->getLogger().info() << "Synchronizing texture ID " << id;
		if (_textures.find(id) == _textures.end()) {
			// It only creates a Albedo texture for now,
			// but it will be extended in the future to support
			// other types of textures (normal, roughness, etc.) based on the
			// material properties.
			_textures[id] =
				std::make_shared<GPUTexture>(*_deviceContext, *texture);
		}
		if (refresh) {
			this->getLogger().info()
				<< "Refreshing GPUTexture for texture ID " << id;
			_textures[id] =
				std::make_shared<GPUTexture>(*_deviceContext, *texture);
		}
	}
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

std::unordered_map<uint32_t, std::shared_ptr<evan::GPUShader>>
	evan::RessourceManager::getShaders() const
{
	return _shaders;
}
