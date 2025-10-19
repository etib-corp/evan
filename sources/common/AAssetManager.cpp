/*
** ETIB PROJECT, 2025
** maverik
** File description:
** AAssetManager
*/

#include "AAssetManager.hpp"

bool maverik::AAssetManager::exists(const std::string &path) const {
  return _assets.find(path) != _assets.end();
}

std::shared_ptr<maverik::FileAsset>
maverik::AAssetManager::get(const std::string &path) {
  if (exists(path)) {
    return _assets[path];
  }
  return nullptr;
}