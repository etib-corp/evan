/*
** EPITECH PROJECT, 2026
** evan
** File description:
** MacOsDesktopPlatform
*/

#include "glfw/platform/MacOsDesktopPlatform.hpp"

evan::MacOsDesktopPlatform::MacOsDesktopPlatform(const std::string &name, const uint32_t width,
                                             const uint32_t height)
{
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	_window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
	if (!_window) {
		throw std::runtime_error("Failed to create GLFW window");
	}
}

std::vector<std::string> evan::MacOsDesktopPlatform::getRequiredInstanceExtensions() const
{
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions =
		glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char *> extensions(glfwExtensions,
										 glfwExtensions + glfwExtensionCount);

	extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	if (enableValidationLayers == true) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	return std::vector<std::string>(extensions.begin(), extensions.end());
}

VkSurfaceKHR evan::MacOsDesktopPlatform::createSurface(VkInstance instance) const
{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, _window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
    return surface;
}