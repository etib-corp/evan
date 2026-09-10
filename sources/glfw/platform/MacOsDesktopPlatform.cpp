/*
** ETIB PROJECT, 2026
** evan
** File description:
** MacOsDesktopPlatform
*/

#include "evan/glfw/platform/MacOsDesktopPlatform.hpp"

#include <stdexcept>

evan::MacOsDesktopPlatform::MacOsDesktopPlatform(const std::string &name,
												 const uint32_t width,
												 const uint32_t height)
{
	this->getLogger().info()
		<< "Initializing MacOsDesktopPlatform with window name: " << name
		<< ", width: " << width << ", height: " << height;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	this->getLogger().info() << "Creating GLFW window";
	_window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
	if (!_window) {
		this->getLogger().error() << "Failed to create GLFW window";
		throw std::runtime_error("Failed to create GLFW window");
	}
}

////////////////////
// Public Methods //
////////////////////

std::vector<std::string>
	evan::MacOsDesktopPlatform::getRequiredInstanceExtensions() const
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

VkInstanceCreateFlags evan::MacOsDesktopPlatform::getInstanceCreateFlags() const
{
	return VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
}

std::vector<std::string>
	evan::MacOsDesktopPlatform::getRequiredDeviceExtensions() const
{
	return { "VK_KHR_portability_subset" };
}

VkSurfaceKHR
	evan::MacOsDesktopPlatform::createSurface(VkInstance instance) const
{
	this->getLogger().info()
		<< "Creating Vulkan surface for MacOsDesktopPlatform";

	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, _window, nullptr, &surface)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan surface");
	}
	this->getLogger().info() << "Vulkan surface created successfully";
	return surface;
}