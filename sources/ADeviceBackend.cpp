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
