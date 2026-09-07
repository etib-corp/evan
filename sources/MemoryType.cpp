/*
** ETIB PROJECT, 2026
** evan
** File description:
** MemoryType
*/

#include "evan/MemoryType.hpp"

evan::Result<uint32_t> evan::findMemoryTypeIndex(
	const VkPhysicalDeviceMemoryProperties &memoryProperties,
	uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i))
			&& (memoryProperties.memoryTypes[i].propertyFlags & properties)
				== properties) {
			return { Error::Ok, i };
		}
	}
	return { Error::RuntimeError, 0 };
}
