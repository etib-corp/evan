/*
** EPITECH PROJECT, 2026
** evan
** File description:
** ADeviceBackend
*/

#include "ADeviceBackend.hpp"

evan::ADeviceBackend::~ADeviceBackend()
{
	vkDestroyDevice(_device, nullptr);
	vkDestroyInstance(_VkInstance, nullptr);
}

std::vector<VkLayerProperties> evan::ADeviceBackend::getAvailableLayers()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	return availableLayers;
}

evan::SwapChainSupportDetails
	evan::ADeviceBackend::querySwapChainSupport(VkPhysicalDevice device,
												VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;
	uint32_t formatCount;
	uint32_t presentModeCount;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
											  &details.capabilities);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
											 nullptr)
		!= VK_SUCCESS)
		return details;
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
											 details.formats.data());
	}

	if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
												  &presentModeCount, nullptr)
		!= VK_SUCCESS)
		return details;
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}
