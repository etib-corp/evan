/*
** ETIB PROJECT, 2026
** evan
** File description:
** LinuxXrPlatform
*/

#include "evan/openxr/platform/LinuxXrPlatform.hpp"

#include <cstdlib>

evan::LinuxXrPlatform::LinuxXrPlatform()
{
	this->getLogger().info() << "LinuxXrPlatform initialized";
}

////////////////////
// Public Methods //
////////////////////

std::vector<std::string>
	evan::LinuxXrPlatform::getRequiredInstanceExtensions() const
{
	return { XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME };
}

const XrBaseInStructure *evan::LinuxXrPlatform::getInstanceCreateInfo() const
{
	return nullptr;
}

///////////////////////
// Protected Methods //
///////////////////////

std::filesystem::path evan::LinuxXrPlatform::getDefaultCacheRoot() const
{
	const char *xdgCache = std::getenv("XDG_CACHE_HOME");
	if (xdgCache != nullptr && xdgCache[0] != '\0') {
		return std::filesystem::path(xdgCache);
	}

	const char *home = std::getenv("HOME");
	if (home == nullptr || home[0] == '\0') {
		return {};
	}
	return std::filesystem::path(home) / ".cache";
}