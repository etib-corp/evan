/*
** ETIB PROJECT, 2026
** evan
** File description:
** IDesktopPlatform
*/

#include "glfw/IDesktopPlatform.hpp"

evan::IDesktopPlatform::~IDesktopPlatform()
{
	glfwDestroyWindow(_window);
	glfwTerminate();
}

std::shared_ptr<evan::ADeviceBackend> evan::IDesktopPlatform::createDeviceBackend() const
{
	return std::make_shared<DesktopBackend>(*this);
}

std::shared_ptr<evan::ASwapchainContext> evan::IDesktopPlatform::createSwapchainContext(
	const DeviceContext &deviceContext) const
{
	return std::make_shared<DesktopSwapchainContext>(deviceContext, _window);
}

////////////////////
// Public Methods //
////////////////////

void evan::IDesktopPlatform::pollEvents(ADeviceBackend &deviceBackend)
{
	glfwPollEvents();
}

std::vector<std::string>
	evan::IDesktopPlatform::getRequiredInstanceExtensions() const
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

bool evan::IDesktopPlatform::shouldClose() const
{
	return glfwWindowShouldClose(_window);
}
