/*
** ETIB PROJECT, 2026
** evan
** File description:
** WindowsPlatoform
*/

#include "glfw/platform/WindowsPlatform.hpp"

evan::WindowsDesktopPlatform::WindowsDesktopPlatform(const std::string &name, const uint32_t width,
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

std::vector<std::string> evan::WindowsDesktopPlatform::getRequiredInstanceExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> extensions(glfwExtensions,
                                         glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers == true) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return std::vector<std::string>(extensions.begin(), extensions.end());
}

VkSurfaceKHR evan::WindowsDesktopPlatform::createSurface(VkInstance instance) const
{
    VkSurfaceKHR surface;
    #if defined(__WIN32__)
    VkWin32SurfaceCreateInfoKHR createInfo {};

	createInfo.sType	 = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd		 = glfwGetWin32Window(_window);
	createInfo.hinstance = GetModuleHandle(nullptr);

	if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}
    #else
    throw std::runtime_error("Unsupported platform for surface creation");
    #endif
    return surface;
}