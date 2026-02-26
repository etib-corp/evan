/*
** EPITECH PROJECT, 2026
** evan
** File description:
** IDeviceBackend
*/

#include "IDeviceBackend.hpp"

evan::QueueFamilyIndices evan::IDeviceBackend::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
    }

    return indices;
}

evan::QueueFamilyIndices evan::IDeviceBackend::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
    }

    return indices;
}

std::vector<VkLayerProperties> evan::IDeviceBackend::getAvailableLayers()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    return availableLayers;
}

bool evan::IDeviceBackend::isDeviceSuitable(VkPhysicalDevice device,
								   VkSurfaceKHR surface,
								   std::vector<const char *> deviceExtensions)
{
	QueueFamilyIndices indices = this->findQueueFamilies(device, surface);
	bool extensionsSupported =
		this->checkDeviceExtensionSupport(device, deviceExtensions);
	bool swapChainAdequate = false;

	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport =
			this->querySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty()
			&& !swapChainSupport.presentModes.empty();
	}
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate
		&& supportedFeatures.samplerAnisotropy;
}

bool evan::IDeviceBackend::checkDeviceExtensionSupport(
	VkPhysicalDevice device, std::vector<const char *> deviceExtensions)
{
	uint32_t extensionCount;
	if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
											 nullptr) != VK_SUCCESS)
		return false;

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
										 availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(),
											 deviceExtensions.end());

	for (const auto &extension: availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

evan::SwapChainSupportDetails
	evan::IDeviceBackend::querySwapChainSupport(VkPhysicalDevice device,
									   VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;
	uint32_t formatCount;
	uint32_t presentModeCount;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
											  &details.capabilities);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
											 nullptr) != VK_SUCCESS)
		return details;
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
											 details.formats.data());
	}

	if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
											  &presentModeCount, nullptr) != VK_SUCCESS)
		return details;
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}

