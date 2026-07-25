/*
** ETIB PROJECT, 2026
** evan
** File description:
** LinuxXrPlatform
*/

#include "evan/openxr/platform/LinuxXrPlatform.hpp"

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