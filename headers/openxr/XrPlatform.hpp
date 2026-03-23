/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrPlatform
*/

#pragma once

#include "IPlatform.hpp"
#include "openxr/XrDeviceBackend.hpp"

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
            /**
             * @brief Struct to hold Android-specific platform data for OpenXR initialization.
             *
             * This structure encapsulates the necessary Android-specific data required for initializing OpenXR on an Android platform, such as the application VM and activity pointers.
             *
             * @note This struct is only relevant for Android platforms and may be ignored or left empty on other platforms.
             */
            struct AndroidPlatformData {
                /**
                 * @brief Pointer to the Android application VM.
                 *
                 * This is typically obtained from the Android Native Activity and is used for OpenXR initialization on Android.
                 */
                void *applicationVM;

                /**
                 * @brief Pointer to the Android application activity.
                 *
                 * This is typically obtained from the Android Native Activity and is used for OpenXR initialization on Android.
                 */
                void *applicationActivity;
            };

            /**
             * @brief Constructor for XrPlatform.
             *
             * This constructor initializes the XrPlatform with the provided Android-specific platform data. It sets up the necessary OpenXR instance creation information based on the Android platform data.
             *
             * @param data The platform-specific data required for OpenXR initialization.
             */
            XrPlatform(const AndroidPlatformData &data);

            /**
             * @brief Default constructor for XrPlatform.
             *
             * This constructor can be used when no platform-specific data is required for OpenXR initialization, or when the platform-specific data will be set later.
             */
            XrPlatform();

            /**
             * @brief Destructor for XrPlatform.
             *
             * Ensures proper cleanup of OpenXR resources.
             */
            ~XrPlatform() override;

            /**
             * @brief Get the required instance extensions for the OpenXR platform.
             *
             * This function returns a vector of strings representing the required instance extensions for the OpenXR platform. These extensions are necessary for proper functionality and compatibility with the OpenXR runtime on the target platform.
             *
             * @return A vector of strings containing the required instance extensions for the OpenXR platform.
             */
            std::vector<std::string> getRequiredInstanceExtensions() const override;

            /**
             * @brief Check if the OpenXR platform should close.
             *
             * This function checks the internal state of the XrPlatform to determine if it should close. This is typically based on events such as session state changes or user input that indicates the application should exit.
             *
             * @return true if the platform should close, false otherwise.
             */
            bool shouldClose() const override;

            /**
             * @brief Poll for OpenXR events and handle them accordingly.
             *
             * This function polls for events from the OpenXR runtime and processes them. It handles events such as session state changes, input events, and other relevant OpenXR events. The function may update internal state based on the events received, such as setting the shouldClose flag when a session state change indicates that the application should exit.
             *
             * @param deviceBackend A reference to the ADeviceBackend, which may be used for handling device-specific operations related to the events being processed.
             *
             * @note This function should be called regularly in the main application loop to ensure that events are processed in a timely manner.
             */
            void pollEvents(ADeviceBackend &deviceBackend) override;

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
			const XrBaseInStructure *getInstanceCreateInfoAndroid() const;

        protected:
            /**
             * @brief Process an OpenXR session state change event.
             *
             * This function handles the XrEventDataSessionStateChanged event, updating the internal state of the XrPlatform based on the new session state. It may set flags such as shouldClose or sessionRunning based on the session state received in the event data.
             *
             * @param eventData The event data containing information about the session state change.
             * @param session The OpenXR session associated with the event.
             *
             * @note This function is typically called from within the pollEvents method when a session state change event is received. It is responsible for ensuring that the XrPlatform responds appropriately to changes in the OpenXR session state.
             */
            void processSessionStateChangedEvent(const XrEventDataSessionStateChanged &eventData, XrSession session);

            /**
             * Flag to indicate if the platform should close.
             *
             * OpenXR specific.
             */
            bool _shouldClose = false;

            /**
             * Flag to track if the session is currently running.
             *
             * This is used to determine if the application should continue running or if it should exit based on the session state changes received from the OpenXR runtime.
             */
            bool _sessionRunning = false;

            #ifdef __ANDROID__
                /**
                 * @brief Android-specific instance creation information for OpenXR.
                 *
                 * This structure holds the necessary information for creating an OpenXR instance on an Android platform, such as the application VM and activity pointers. It is initialized based on the AndroidPlatformData provided to the constructor and is used when creating the OpenXR instance to ensure proper initialization on Android.
                 */
                XrInstanceCreateInfoAndroidKHR _instanceCreateInfoAndroid{};
            #endif
    };
} // namespace evan