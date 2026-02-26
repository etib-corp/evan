/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopPlatform
*/

#include "glfw/DesktopPlatform.hpp"

evan::DesktopPlatform::DesktopPlatform(const std::string &name, const uint32_t width, const uint32_t height)
{
    if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	_window =
		glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
	if (!_window) {
		throw std::runtime_error("Failed to create GLFW window");
	}
}

evan::DesktopPlatform::~DesktopPlatform()
{
	glfwDestroyWindow(_window);
	glfwTerminate();
}

std::vector<std::string> evan::DesktopPlatform::getRequiredInstanceExtensions() const
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
	return std::vector<std::string>(extensions.begin(), extensions.end());
}

bool evan::DesktopPlatform::shouldClose() const
{
    return glfwWindowShouldClose(_window);
}

void evan::DesktopPlatform::pollEvents()
{
    glfwPollEvents();
}
