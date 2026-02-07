/*
** ETIB PROJECT, 2025
** evan
** File description:
** AssetsManager
*/

#pragma once

#include "AAssetManager.hpp"
#include <iostream>

/**
 * @namespace evan
 * @brief The evan namespace contains classes and functions for the evan
 * project.
 */
namespace evan
{
	/**
	 * @namespace glfw
	 * @brief The glfw namespace contains classes and functions for the Vulkan
	 * graphics API.
	 */
	namespace glfw
	{
		/**
		 * @class AssetsManager
		 * @brief The AssetsManager class is responsible for managing file
		 * assets.
		 *
		 * This class inherits from AAssetManager and implements the addAsset
		 * method to load assets from files.
		 */
		class AssetsManager: public AAssetManager
		{
			public:
			/**
			 * @brief Default constructor for AssetsManager.
			 */
			AssetsManager() = default;

			/**
			 * @brief Default destructor for AssetsManager.
			 */
			~AssetsManager() override = default;

			/**
			 * @brief Adds an asset to the manager.
			 * @param path The path to the asset.
			 * @return A shared pointer to the FileAsset object. If the file
			 * can't be opened, it returns a nullptr and logs an error message.
			 */
			std::shared_ptr<evan::FileAsset>
				add(const std::string &path) override;

			/**
			 * @brief Removes an asset from the manager.
			 * @param path The path to the asset.
			 *
			 * This method removes the asset from the _assets map.
			 * Using a deleted asset will result in undefined behavior.
			 */
			void remove(const std::string &path, bool save = true) override;

			/**
			 * @brief Saves an asset to the disk.
			 * @param path The path to the asset.
			 * @param newPath The new path to save the asset to (optional).
			 * @return True if the asset was saved successfully, false
			 * otherwise.
			 *
			 * This method is pure virtual and must be implemented by derived
			 * classes. It is responsible for saving the content of the asset to
			 * the specified path. If newPath is provided, it will save the
			 * asset to that path and will not update the original path. If
			 * newPath is not provided, it will save the asset to the original
			 * path.
			 */
			bool save(const std::string &path,
					  const std::string &newPath = "") override;
		};
	}	 // namespace glfw
}	 // namespace evan