/*
** ETIB PROJECT, 2025
** evan
** File description:
** AAssetManager
*/

#include "AAssetManager.hpp"

bool evan::AAssetManager::exists(const std::string &path) const {
  return _assets.find(path) != _assets.end();
}

std::shared_ptr<evan::FileAsset>
evan::AAssetManager::get(const std::string &path) {
  if (exists(path)) {
    return _assets[path];
  }
  return nullptr;
}