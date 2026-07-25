/*
** ETIB PROJECT, 2026
** evan
** File description:
** LinuxDesktopPlatform
*/

#include "evan/glfw/platform/LinuxDesktopPlatform.hpp"

evan::LinuxDesktopPlatform::LinuxDesktopPlatform(const std::string &name,
												 const uint32_t width,
												 const uint32_t height)
{
	this->getLogger().info()
		<< "Initializing LinuxDesktopPlatform with window name: " << name
		<< ", width: " << width << ", height: " << height;

	if (!glfwInit()) {
		this->getLogger().error() << "Failed to initialize GLFW";
		return;
	}

	this->getLogger().info() << "GLFW initialized successfully";

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	this->getLogger().info() << "Creating GLFW window";
	_window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

	if (!_window) {
		this->getLogger().error() << "Failed to create GLFW window";
		return;
	}
	glfwSetWindowAspectRatio(_window, width, height);
}

////////////////////
// Public Methods //
////////////////////

std::vector<std::string>
	evan::LinuxDesktopPlatform::getRequiredInstanceExtensions() const
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

VkSurfaceKHR
	evan::LinuxDesktopPlatform::createSurface(VkInstance instance) const
{
	this->getLogger().info()
		<< "Creating Vulkan surface for LinuxDesktopPlatform";

	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, _window, nullptr, &surface)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to create Vulkan surface";
		return VK_NULL_HANDLE;
	}
	this->getLogger().info() << "Vulkan surface created successfully";
	return surface;
}