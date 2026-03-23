/*
** ETIB PROJECT, 2026
** evan
** File description:
** QueueFamilyIndices
*/

#pragma once

#include <optional>

namespace evan
{
	/**
	 * @brief A structure to represent Vulkan queue family indices.
	 *
	 * This structure is used to store the indices of queue families
	 * that support specific operations, such as graphics and presentation.
	 *
	 */
	struct QueueFamilyIndices {
		/*
		 * An optional value representing the index of the queue family
		 * that supports graphics operations.
		 */
		std::optional<uint32_t> graphicsFamily;

		/*
		 * An optional value representing the index of the queue family
		 * that supports presentation operations.
		 */
		std::optional<uint32_t> presentFamily;

		/*
		 * @brief isComplete
		 *
		 * A method to check if both graphicsFamily and presentFamily
		 * have been assigned values.
		 *
		 * @return true if both graphicsFamily and presentFamily have
		 * values, indicating that the required queue families are
		 * available. False otherwise.
		 */
		bool isComplete() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};
}	 // namespace evan
