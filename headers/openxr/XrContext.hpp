/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrContext
*/

#pragma once

#include "openxr/XrPlatform.hpp"

#include <iostream>

namespace evan {

    class XrContext {
        public:
            /**
             * @brief Constructor for XrContext.
             *
             * Initializes the OpenXR context with the provided platform.
             *
             *
             */
            XrContext(const XrPlatform &platform);

            /**
             * @brief Destructor for XrContext.
             *
             * Ensures proper cleanup of OpenXR resources.
             */
            ~XrContext();

        private:
            /**
             * Creates an OpenXR instance with the specified platform configuration.
             *
             * This function initializes an OpenXR instance by:
             * - Collecting required extensions (including XR_KHR_VULKAN_ENABLE2 and platform-specific extensions)
             * - Configuring instance creation parameters with the provided platform settings
             * - Creating the XR instance and storing it in the _instance member variable
             *
             * @param platform The XrPlatform object containing platform-specific initialization data
             *
             * @note On Android, the XR_KHR_ANDROID_CREATE_INSTANCE extension is automatically added
             *       and platform-specific creation info is attached to the instance creation info.
             *
             * @see XrPlatform, XrInstanceCreateInfo, xrCreateInstance
             */
            void createInstance(const XrPlatform &platform);

            /**
             * @brief Initializes the OpenXR system by retrieving the system ID for a head-mounted display.
             *
             * This function queries the OpenXR runtime to obtain a system ID configured for a
             * head-mounted display (HMD) form factor. The system ID is stored in the _systemId
             * member variable for later use in creating sessions and querying device capabilities.
             *
             * @note Error handling is incomplete (see TODO). Consider implementing proper exception
             *       handling or a more robust error reporting mechanism in future versions.
             *
             * @see XrSystemGetInfo, xrGetSystem()
             */
            void initializeSystem();

            XrInstance _instance = XR_NULL_HANDLE;      // OpenXR instance handle
            XrSystemId _systemId = XR_NULL_SYSTEM_ID;   // OpenXR system ID
    };
} // namespace evan