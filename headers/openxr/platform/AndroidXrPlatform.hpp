/*
** ETIB PROJECT, 2026
** evan
** File description:
** AndroidXrPlatform
*/

#pragma once

#include "openxr/IXrPlatform.hpp"

namespace evan
{
    /**
     * @brief The AndroidXrPlatform class provides platform-specific implementations
     * for OpenXR on Android devices.
     *
     * This class implements the IXrPlatform interface to provide the necessary
     * configurations and extensions required for creating an OpenXR instance on
     * Android. It handles the retrieval of required instance extensions and
     * provides the appropriate instance creation information for Android platforms.
     */
    class AndroidXrPlatform: public IXrPlatform
    {
        public:
        /**
		 * @brief Struct to hold Android-specific platform data for OpenXR
		 * initialization.
		 *
		 * This structure encapsulates the necessary Android-specific data
		 * required for initializing OpenXR on an Android platform, such as the
		 * application VM and activity pointers.
		 *
		 * @note This struct is only relevant for Android platforms and may be
		 * ignored or left empty on other platforms.
		 */
        struct AndroidPlatformData {
			/**
			 * @brief Pointer to the Android application VM.
			 *
			 * This is typically obtained from the Android Native Activity and
			 * is used for OpenXR initialization on Android.
			 */
			void *applicationVM;

			/**
			 * @brief Pointer to the Android application activity.
			 *
			 * This is typically obtained from the Android Native Activity and
			 * is used for OpenXR initialization on Android.
			 */
			void *applicationActivity;

            /**
             * @brief Pointer to the Android application structure.
             * This is used for handling Android application lifecycle events and may
             * be necessary for certain OpenXR implementations on Android.
             */
            void *androidApp;
		};

        /**
         * @brief Struct to track the current state of the Android application.
         * This struct is used to manage the lifecycle of the Android application and
         * ensure that the OpenXR session responds appropriately to changes in the application state,
         * such as when the app is resumed, paused, or stopped.
         */
        struct AndroidAppState {
            #ifdef __ANDROID__
                /**
                 * @brief Pointer to the native Android window.
                 *
                 * This is typically obtained from the Android Native Activity and
                 * is used for rendering and handling input in OpenXR on Android.
                 */
                ANativeWindow* NativeWindow = nullptr;
            #endif

            /**
             * @brief Indicates whether the Android application is currently resumed.
             * This flag can be used to determine if the OpenXR session should be active
             * or if it should pause rendering and input processing when the app is not in the foreground.
             */
            bool Resumed = false;
        };


        /**
         * @brief Constructs an AndroidXrPlatform instance with the specified
         * Android platform data.
         *
         * Initializes the AndroidXrPlatform with the provided Android-specific
         * data, which is necessary for creating an OpenXR instance on Android
         * platforms.
         *
         * @param platformData The Android-specific platform data required for OpenXR
         * initialization on Android devices.
         */
        AndroidXrPlatform(const AndroidPlatformData &platformData);

        /**
         * @brief Destructor for AndroidXrPlatform.
         *
         * Ensures proper cleanup of resources used by the AndroidXrPlatform.
         */
        ~AndroidXrPlatform() override = default;

        /**
         * @brief Get the required instance extensions for the OpenXR platform on
         * Android.
         *
         * This function returns a vector of strings representing the required
         * instance extensions for the OpenXR platform on Android. These
         * extensions are necessary for proper functionality and compatibility with
         * the OpenXR runtime on Android devices.
         *
         * @return A vector of strings containing the required instance extensions
         * for the OpenXR platform on Android.
         */
        void pollEvents(ADeviceBackend &deviceBackend) override;

        /**
         * @brief Retrieves the required instance extensions for Android in the OpenXR
         * device backend.
         *
         * This function filters and returns a list of available Vulkan layers
         * that match the KHRONOS validation layer. It is specifically used to
         * configure the OpenXR instance with the appropriate validation layers
         * on Android platforms.
         *
         */
        std::vector<std::string> getRequiredInstanceExtensions() const override;

        /**
         * @brief Retrieves the instance creation information specific to Android.
         *
         * This method constructs and returns a pointer to an XrInstanceCreateInfo
         * structure that contains the necessary information for creating an OpenXR
         * instance on Android. It may include platform-specific extensions or
         * configurations required by the OpenXR runtime on Android devices.
         *
         * @return const void* A pointer to the XrInstanceCreateInfo structure
         * containing the instance creation information for Android. The caller is
         * responsible for ensuring that the returned pointer remains valid for the
         * duration of its use.
         */
        const XrBaseInStructure *getInstanceCreateInfo() const override;

        /**
         * @brief Tracks the current state of the Android application.
         *
         * This variable is used to keep track of the current state of the
         * Android application, such as whether it is in the foreground, paused,
         * or stopped. It can be used to manage OpenXR session state and ensure
         * that the application responds appropriately to changes in its lifecycle
         * on Android devices.
         */
        AndroidAppState _appState = {};

    private:
    #ifdef __ANDROID__
        /**
         * @brief Android-specific instance creation information for OpenXR.
         *
         * This structure holds the necessary information for creating an OpenXR
         * instance on an Android platform, such as the application VM and
         * activity pointers. It is initialized based on the AndroidPlatformData
         * provided to the constructor and is used when creating the OpenXR
         * instance to ensure proper initialization on Android.
         */
        XrInstanceCreateInfoAndroidKHR _instanceCreateInfoAndroid {};
    #endif

        /**
         * @brief Stores the Android-specific platform data for OpenXR initialization.
         *
         * This member variable holds the Android-specific data provided to the
         * constructor, which is necessary for initializing OpenXR on Android
         * platforms. It may include pointers to the application VM, activity, and
         * other relevant Android data required for proper OpenXR functionality on
         * Android devices.
         */
        AndroidPlatformData _platformData;
    };
}