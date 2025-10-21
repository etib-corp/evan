/*
** ETIB PROJECT, 2025
** evan
** File description:
** AAssetManager
*/

#pragma once

#include "FileAsset.hpp"

#include <map>
#include <memory>
#include <string>
#include <utility>

/**
 * @namespace evan
 * @brief The evan namespace contains classes and functions for the evan
 * project.
 */
namespace evan {
/**
 * @class AAssetManager
 * @brief The AAssetManager class is an abstract base class for managing file
 * assets.
 */
class AAssetManager {
public:
  /**
   * @brief Defautl destructor for AAssetManager.
   */
  virtual ~AAssetManager() = default;

  /**
   * @brief Adds an asset to the manager.
   * @param path The path to the asset.
   * @return A shared pointer to the FileAsset object.
   *
   * This method is pure virtual and must be implemented by derived classes.
   * This method is responsible for loading the asset from the specified path
   * by reading the file content and storing it in the _assets map.
   */
  virtual std::shared_ptr<evan::FileAsset> add(const std::string &path) = 0;

  /**
   * @brief Removes an asset from the manager.
   * @param path The path to the asset.
   * @param save Whether to save the asset before removing it (default is true).
   *
   * This method removes the asset from the _assets map.
   * When removing an asset, the asset file will not be deleted from the disk.
   * When removing an asset, its content is updated on the disk if save is true.
   * If save is false, the asset will be removed from the _assets map without
   * saving its content on the disk. Using a deleted asset will result in
   * undefined behavior.
   */
  virtual void remove(const std::string &path, bool save = true) = 0;

  /**
   * @brief Saves an asset to the disk.
   * @param path The path to the asset.
   * @param newPath The new path to save the asset to (optional).
   * @return True if the asset was saved successfully, false otherwise.
   *
   * This method is pure virtual and must be implemented by derived classes.
   * It is responsible for saving the content of the asset to the specified
   * path. If newPath is provided, it will save the asset to that path and will
   * not update the original path. If newPath is not provided, it will save the
   * asset to the original path.
   */
  virtual bool save(const std::string &path,
                    const std::string &newPath = "") = 0;

  /**
   * @brief Checks if an asset exists in the manager.
   * @param path The path to the asset.
   * @return True if the asset exists, false otherwise.
   *
   * This method checks if the asset is present in the _assets map.
   */
  bool exists(const std::string &path) const;

  /**
   * @brief Gets an asset from the manager.
   * @param path The path to the asset.
   * @return A shared pointer to the FileAsset object.
   *
   * This method retrieves the asset from the _assets map.
   * If the asset does not exist, it returns a nullptr.
   */
  std::shared_ptr<evan::FileAsset> get(const std::string &path);

protected:
  std::map<std::string, std::shared_ptr<evan::FileAsset>>
      _assets; ///> The map of assets, where the key is the path and the value
               ///is a shared pointer to the FileAsset object.
};
} // namespace evan
