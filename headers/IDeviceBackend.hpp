/*
** EPITECH PROJECT, 2026
** evan
** File description:
** IDeviceBackend
*/

#pragma once

#include "EvanPlatform.hpp"\

#include <optional>

namespace evan {

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

    class IDeviceBackend {
        public:
            virtual ~IDeviceBackend() = default;

            virtual VkInstance createInstance() = 0;
            
            virtual VkDevice createLogicalDevice() = 0;

            virtual VkPhysicalDevice pickPhysicalDevice() = 0;

			/**
 			* @brief Finds the queue families supported by a given Vulkan physical device.
 			*
 			* This function iterates through the queue families of the specified physical
 			* device and identifies the indices of queue families that support specific
 			* capabilities, such as graphics operations.
 			*
 			* @param device The Vulkan physical device to query for queue family
 			* properties.
 			* @return A QueueFamilyIndices structure containing the indices of the queue
 			* families that meet the required criteria. If no suitable queue families are
 			* found, the indices will remain unset.
 			*/
			evan::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

			/**
			 * @brief Finds the queue families that support specific operations on a given
			 * Vulkan physical device and surface.
			 *
			 * This function identifies the queue families that support graphics operations
			 * and presentation to a given surface. It iterates through the queue families
			 * of the specified physical device and checks their capabilities.
			 *
			 * @param device The Vulkan physical device to query for queue family
			 * properties.
			 * @param surface The Vulkan surface to check for presentation support.
			 * @return QueueFamilyIndices A structure containing the indices of the graphics
			 * and presentation queue families. If no suitable queue families are found, the
			 * indices will remain incomplete.
			 */
			evan::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    }; 
} // namespace evan