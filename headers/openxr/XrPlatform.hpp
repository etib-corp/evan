/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrPlatform
*/

#pragma once

#include "IPlatform.hpp"

#include <iostream>

namespace evan {
    /**
     * @brief OpenXR platform implementation.
     * 
     * This class provides an implementation of the IPlatform interface for
     * OpenXR. It handles platform-specific functionality related to OpenXR.
     */
    class XrPlatform : public IPlatform {
        public:
            struct AndroidPlatformData {
                void *applicationVM;
                void *applicationActivity;
            };

            /**
             * @brief Constructor for XrPlatform.
             * 
             * @param data The platform-specific data required for OpenXR initialization.
             */
            XrPlatform(const AndroidPlatformData &data);

            XrPlatform();

            /**
             * @brief Destructor for XrPlatform.
             * 
             * Ensures proper cleanup of OpenXR resources.
             */
            ~XrPlatform() override;

            std::vector<std::string> getRequiredInstanceExtensions() const override;

            bool shouldClose() const override;

            void pollEvents() override;

            /**
			 * @brief Retrieves the Android-specific instance creation
			 * information.
			 *
			 * This function returns a pointer to the Android-specific instance
			 * creation structure, cast to a generic XrBaseInStructure pointer.
			 * This is typically used when creating an OpenXR instance on an
			 * Android platform.
			 *
			 * @return A pointer to the Android-specific instance creation
			 * information as an XrBaseInStructure.
			 */
			XrBaseInStructure *getInstanceCreateInfoAndroid();
        
        protected:
            bool _shouldClose = false; // Flag to indicate if the platform should close

            XrInstanceCreateInfoAndroidKHR _instanceCreateInfoAndroid = 
    };
} // namespace evan