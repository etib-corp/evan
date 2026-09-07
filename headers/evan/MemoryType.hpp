/*
** ETIB PROJECT, 2026
** evan
** File description:
** MemoryType
*/

#pragma once

#include "evan/EvanPlatform.hpp"
#include "evan/Error.hpp"

namespace evan
{
	/**
	 * @brief Finds a Vulkan memory type index satisfying the given constraints.
	 *
	 * Pure helper that resolves a memory type index from physical device memory
	 * properties without touching the device. It returns an error instead of
	 * silently falling back to memory type 0 when no memory type matches the
	 * type filter and requested property flags.
	 *
	 * @param memoryProperties The physical device memory properties to search.
	 * @param typeFilter Bitmask of acceptable memory type indices.
	 * @param properties Required VkMemoryPropertyFlags.
	 *
	 * @return Result<uint32_t> Index of the matching memory type, or an error.
	 */
	[[nodiscard]] Result<uint32_t> findMemoryTypeIndex(
		const VkPhysicalDeviceMemoryProperties &memoryProperties,
		uint32_t typeFilter, VkMemoryPropertyFlags properties);
}	 // namespace evan
