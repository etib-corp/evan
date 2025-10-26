/*
** ETIB PROJECT, 2025
** evan
** File description:
** AssetsManager
*/

#include "glfw/AssetsManager.hpp"

std::shared_ptr<evan::FileAsset>
evan::glfw::AssetsManager::add(const std::string &path) {
  if (this->exists(path)) {
    return _assets[path];
  }
  std::ifstream file(path, std::ios::binary);

  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << path << std::endl;
    return nullptr;
  }
  std::string content;
  file.seekg(0, std::ios::end);
  content.resize(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(&content[0], content.size());
  file.close();
  _assets[path] = std::make_shared<evan::FileAsset>(content);
  if (!_assets[path]) {
    std::cerr << "Failed to create FileAsset for: " << path << std::endl;
    return nullptr;
  }
  return _assets[path];
}

void evan::glfw::AssetsManager::remove(const std::string &path, bool save) {
  auto it = _assets.find(path);
  if (it != _assets.end()) {
    if (save) {
      this->save(path);
    }
    _assets.erase(it);
  } else {
    std::cerr << "Asset not found: " << path << std::endl;
  }
}

bool evan::glfw::AssetsManager::save(const std::string &path,
                                     const std::string &newPath) {
  auto it = _assets.find(path);
  if (it == _assets.end()) {
    std::cerr << "Asset not found: " << path << std::endl;
    return false;
  }
  std::string savePath = newPath.empty() ? path : newPath;
  std::ofstream file(savePath, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to open file for saving: " << savePath << std::endl;
    return false;
  }
  file.write(it->second->content().data(), it->second->content().size());
  file.close();
  return true;
}