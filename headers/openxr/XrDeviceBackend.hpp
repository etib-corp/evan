/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrBackend
*/

#pragma once

#include "openxr/XrPlatform.hpp"

#include "IDeviceBackend.hpp"

#include <cstring>
#include <iostream>
#include <openxr/openxr.h>
#include <vector>

namespace evan
{
    class XrDeviceBackend : public IDeviceBackend {
        public:
            XrDeviceBackend();
            ~XrDeviceBackend() override;

            void init(const IPlatform& platform) override;

            VkInstance createInstance(const IPlatform& platform, const std::string &appName, Version &appVersion) override;

            VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice) override;

            VkPhysicalDevice pickPhysicalDevice(VkInstance instance) override;

        protected:

            /**
            * @brief Creates an OpenXR instance with platform-specific extensions and configurations.
            *
            * Initializes the XrInstance by:
            * - Collecting required instance extensions from the platform
            * - Configuring XrInstanceCreateInfo with the extensions
            * - Setting platform-specific creation parameters via Android-specific info
            * - Creating the XR instance and logging any errors
            *
            * @param platform Reference to the IPlatform implementation providing required extensions
            *                 and platform-specific OpenXR instance creation information
            *
            * @note The function uses dynamic_cast which may return nullptr. Consider adding null
            *       pointer checks before dereferencing the casted pointer.
            *
            * @see xrCreateInstance
            * @see XrInstanceCreateInfo
            */
            void createXrInstance(const IPlatform& platform);

            /**
             * @brief Retrieves the OpenXR system ID for the head-mounted display form factor.
             *
             * Initializes the system information structure with the appropriate form factor
             * (XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY) and queries the OpenXR instance for a
             * compatible system. The retrieved system ID is stored in the member variable
             * _systemId for later use.
             *
             * @see xrGetSystem
             * @see XrSystemGetInfo
             */
            void getSystem();

            void createSession(VkDevice device);

            /**
            * Retrieves the list of Vulkan instance extensions available for the validation layer.
            *
            * This function enumerates all instance extension properties supported by the
            * VK_LAYER_KHRONOS_validation layer. It performs two queries: first to determine
            * the number of available extensions, then to retrieve the full extension list.
            *
            * @return A vector of VkExtensionProperties containing all available instance extensions.
            *         Returns an empty vector if enumeration fails at any step.
            *
            * @note The function queries extensions specifically for the "VK_LAYER_KHRONOS_validation" layer.
            */
            std::vector<VkExtensionProperties> getInstanceExtensions() const;

            /**
            * @brief Retrieves the required Vulkan instance extensions for OpenXR.
            *
            * Queries available instance extensions and filters them to return only those
            * that are required for debugging and validation purposes. Specifically, this
            * method identifies and returns the following extensions if available:
            * - VK_EXT_DEBUG_UTILS_EXTENSION_NAME: For debug utility functions
            * - VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME: For validation layer features
            *
            * @return std::vector<const char*> A vector of C-strings containing the names
            *         of required instance extensions that are available on the current system.
            *         The vector may be empty if none of the required extensions are available.
            *
            * @note The returned pointers are valid only while the extension data remains
            *       in memory. Callers should not assume the pointers remain valid after
            *       subsequent calls to getInstanceExtensions().
            */
            std::vector<const char*> getRequiredInstanceExtensions() const;

            /**
            * Retrieves the required instance extensions for Android in the OpenXR device backend.
            *
            * This function filters and returns a list of available Vulkan layers that match
            * the KHRONOS validation layer. It is specifically used to configure the OpenXR
            * instance with the appropriate validation layers on Android platforms.
            *
            */
            std::vector<const char*> getRequiredInstanceExtensionsAndroid();

            XrInstance _XrInstance;    /// The OpenXR instance, which represents the connection between the application and the OpenXR runtime. It is used to create and manage OpenXR resources and to query the capabilities of the OpenXR implementation.
            XrSystemId _systemId;    /// The OpenXR system ID, which represents the specific XR system (e.g., VR headset) that the application is targeting. It is used to query the capabilities of the XR system and to create sessions for rendering.
            XrSession _session;    /// The OpenXR session, which represents an active connection to the XR system. It is used to manage the lifecycle of the XR experience and to submit rendering commands for display on the XR device.
    };
} // namespace evan