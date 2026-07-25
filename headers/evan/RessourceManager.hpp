/*
** ETIB PROJECT, 2026
** xider
** File description:
** RessourceManager
*/

#pragma once

#include <memory>
#include <unordered_map>

#include <utility/ressource_provider.hpp>
#include <utility/graphic/material.hpp>
#include <utility/graphic/texture.hpp>

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include "GPUMaterial.hpp"
#include "GPUTexture.hpp"
#include "GPUShader.hpp"

namespace evan
{
	/**
	 * @brief The RessourceManager class is responsible for managing GPU
	 * resources such as materials and textures.
	 *
	 * The RessourceManager interacts with the utility::RessourceProvider to
	 * load materials and textures, and it creates corresponding GPU resources
	 * (GPUMaterial and GPUTexture) for use in rendering. It maintains internal
	 * maps to store these GPU resources for efficient retrieval based on their
	 * IDs.
	 */
	class RessourceManager:
		protected utility::logging::Loggable<RessourceManager,
											 utility::logging::DefaultLogger>
	{
		public:
		/**
		 * @brief Constructs a RessourceManager instance with the given resource
		 * provider, device context, and renderer.
		 *
		 * @param ressourceProvider A shared pointer to a
		 * utility::RessourceProvider instance used to load materials and
		 * textures.
		 * @param deviceContext A shared pointer to a DeviceContext instance
		 * used to create GPU resources.
		 *
		 * @note The constructor initializes the resource manager and
		 * synchronizes the GPU resources with the loaded materials and textures
		 * from the resource provider.
		 */
		RessourceManager(
			std::shared_ptr<utility::RessourceProvider> ressourceProvider,
			std::shared_ptr<DeviceContext> deviceContext);

		/**
		 * @brief Destructs the RessourceManager instance and cleans up GPU
		 * resources.
		 */
		~RessourceManager();

		/**
		 * @brief Initializes the RessourceManager by synchronizing GPU
		 * resources with the materials and textures loaded from the resource
		 * provider.
		 *
		 * This method should be called after constructing the RessourceManager
		 * to ensure that all GPU resources are properly initialized and
		 * synchronized with the loaded materials and textures. It may also be
		 * called to refresh the GPU resources if there are changes in the
		 * loaded materials or textures.
		 *
		 * @param renderer A shared pointer to the Renderer instance that may be
		 * used for resource initialization and management during the
		 * synchronization process. This allows the RessourceManager to interact
		 * with the Renderer as needed to ensure that GPU resources are properly
		 * set up for rendering operations.
		 */
		void init(std::shared_ptr<Renderer> renderer);

		/**
		 * @brief Synchronizes the GPU resources with the materials and textures
		 * loaded from the resource provider.
		 *
		 * This method checks the materials and textures loaded in the resource
		 * provider and creates corresponding GPU resources for any new
		 * materials or textures that have been added since the last
		 * synchronization. If the `refresh` parameter is set to true, it will
		 * also destroy and recreate GPU resources for materials and textures
		 * that have been modified.
		 *
		 * @param refresh A boolean flag indicating whether to refresh existing
		 * GPU resources for modified materials and textures. Default is false.
		 *
		 * @note This method should be called whenever there are changes in the
		 * loaded materials or textures to ensure that the GPU resources are up
		 * to date.
		 */
		void sync(bool refresh = false);

		/**
		 * @brief Retrieves a shared pointer to the GPUMaterial associated with
		 * the given material ID.
		 *
		 * @param id The unique identifier for the material resource.
		 *
		 * @return A shared pointer to the corresponding GPUMaterial if found,
		 * or nullptr if not found.
		 */
		std::shared_ptr<GPUMaterial> getMaterial(uint32_t id) const;

		/**
		 * @brief Retrieves a shared pointer to the GPUTexture associated with
		 * the given texture ID.
		 *
		 * @param id The unique identifier for the texture resource.
		 *
		 * @return A shared pointer to the corresponding GPUTexture if found, or
		 * nullptr if not found.
		 */
		std::shared_ptr<GPUTexture> getTexture(uint32_t id) const;

		/**
		 * @brief Retrieves a shared pointer to the GPUShader associated with
		 * the given shader ID.
		 *
		 * @param id The unique identifier for the shader resource.
		 *
		 * @return A shared pointer to the corresponding GPUShader if found, or
		 * nullptr if not found.
		 */
		std::shared_ptr<GPUShader> getShader(uint32_t id) const;

		/**
		 * @brief Retrieves a map of all GPUShaders managed by the
		 * RessourceManager.
		 *
		 * @return An unordered map where the key is the unique shader ID and
		 * the value is a shared pointer to the corresponding GPUShader.
		 */
		std::unordered_map<uint32_t, std::shared_ptr<GPUShader>>
			getShaders() const;

		protected:
		/**
		 * @brief Internal method to retrieve the next unique ID for a resource.
		 *
		 * This method can be used to generate unique IDs for GPU resources
		 * based on the materials and textures loaded from the resource
		 * provider.
		 */
		std::shared_ptr<utility::RessourceProvider> _ressourceProvider;

		/**
		 * @brief Shared pointer to the DeviceContext used for creating GPU
		 * resources.
		 *
		 * This member variable holds a shared pointer to the DeviceContext
		 * instance, which provides access to Vulkan device resources and
		 * configurations needed for creating GPU materials and textures. It is
		 * used throughout the RessourceManager to ensure that GPU resources are
		 * created with the correct device context.
		 */
		std::shared_ptr<DeviceContext> _deviceContext;

		/**
		 * @brief Shared pointer to the Renderer instance.
		 *
		 * This member variable holds a shared pointer to the Renderer instance,
		 * which may be used for resource initialization and management.
		 */
		std::shared_ptr<Renderer> _renderer;

		/**
		 * @brief Internal maps to store GPU materials and textures for
		 * efficient retrieval based on their unique IDs.
		 */
		std::unordered_map<uint32_t, std::shared_ptr<GPUMaterial>> _materials;

		/**
		 * @brief Internal map to store GPU textures for efficient
		 * retrieval based on their unique IDs.
		 */
		std::unordered_map<uint32_t, std::shared_ptr<GPUTexture>> _textures;

		/**
		 * @brief Internal map to store GPU shaders for efficient
		 * retrieval based on their unique IDs.
		 */
		std::unordered_map<uint32_t, std::shared_ptr<GPUShader>> _shaders;
	};
}	 // namespace evan
