/*
** EPITECH PROJECT, 2026
** evan
** File description:
** ADeviceBackend
*/

#pragma once

#include "EvanPlatform.hpp"
#include "Version.hpp"

#include "IPlatform.hpp"
#include "QueueFamilyIndices.hpp"

#include <set>

namespace evan
{
	class ADeviceBackend
	{
		public:
		virtual ~ADeviceBackend();

		virtual void createInstance(const IPlatform &platform,
									const std::string &appName,
									Version &appVersion) = 0;

		virtual uint32_t countSwapchainFormats() const = 0;
		virtual std::vector<int64_t> enumerateSwapchainFormats(uint32_t swapchainFormatCount) const = 0;

		virtual void createLogicalDevice() = 0;

		virtual void pickPhysicalDevice() = 0;

		/**
		 * @brief Finds the queue family indices for the Vulkan physical device.
		 *
		 * This function queries the Vulkan API to find the indices of the queue
		 * families that support graphics and presentation operations. It
		 * returns a QueueFamilyIndices structure containing the indices of the
		 * graphics and presentation queue families, if they are found. If the
		 * required queue families are not found, the corresponding optional
		 * values in the QueueFamilyIndices structure will be empty.
		 */
		virtual evan::QueueFamilyIndices findQueueFamilies() = 0;

		/**
		 * @brief Retrieves the available Vulkan layers on the system.
		 *
		 * This function queries the Vulkan API to enumerate the available
		 * instance layers and returns a vector of VkLayerProperties structures
		 * containing information about each layer.
		 */
		std::vector<VkLayerProperties> getAvailableLayers();

		VkInstance
			_VkInstance;	/// The Vulkan instance, which is the connection
							/// between the application and the Vulkan library.
							/// It is used to create and manage Vulkan resources
							/// and to query the capabilities of the Vulkan
							/// implementation.
		VkPhysicalDevice
			_physicalDevice;	/// The physical device, which represents a
								/// Vulkan-compatible GPU. It is used to query
								/// the capabilities of the GPU and to create
								/// logical devices for rendering operations.
		VkDevice _device;		/// The logical device, which represents an
								/// instance of a physical device and is used to
								/// perform rendering operations.
		private:

	};
}	 // namespace evan