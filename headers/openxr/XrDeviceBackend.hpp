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

            VkInstance createInstance(const IPlatform& platform) override;

            VkDevice createLogicalDevice() override;

            VkPhysicalDevice pickPhysicalDevice() override;

        protected:

            void createXrInstance(const IPlatform& platform);

            void getSystem();

            void createSession();

            std::vector<VkExtensionProperties> getInstanceExtensions() const;

            std::vector<const char*> getRequiredInstanceExtensions() const;

            XrInstance _XrInstance;    /// The OpenXR instance, which represents the connection between the application and the OpenXR runtime. It is used to create and manage OpenXR resources and to query the capabilities of the OpenXR implementation.
            XrSystemId _systemId;    /// The OpenXR system ID, which represents the specific XR system (e.g., VR headset) that the application is targeting. It is used to query the capabilities of the XR system and to create sessions for rendering.
            XrSession _session;    /// The OpenXR session, which represents an active connection to the XR system. It is used to manage the lifecycle of the XR experience and to submit rendering commands for display on the XR device.
    };
} // namespace evan