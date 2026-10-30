/*
** ETIB PROJECT, 2026
** evan
** File description:
** LinuxXrPlatform
*/

#include "evan/openxr/platform/LinuxXrPlatform.hpp"
#include "evan/openxr/XrExtensions.hpp"

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
	const std::vector<std::string> baseline = {
		XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME
	};

	if (!_options.enableArExtensions) {
		return baseline;
	}

	const auto available = XrExtensions::enumerateInstanceExtensions();
	if (available.empty()) {
		this->getLogger().warning()
			<< "Could not enumerate OpenXR instance extensions; AR "
			   "extensions stay off.";
		return baseline;
	}

	for (const auto name:
		 XrExtensions::extensionsForFeatures(_options.minimumArFeatures)) {
		if (!XrExtensions::isExtensionSupported(available, name)) {
			this->getLogger().info() << "OpenXR AR extension " << name
									 << " is not available; skipping.";
		}
	}

	return XrExtensions::composeRequiredInstanceExtensions(
		baseline, true, _options.minimumArFeatures, available);
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