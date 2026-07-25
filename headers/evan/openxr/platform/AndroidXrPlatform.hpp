/*
** ETIB PROJECT, 2026
** evan
** File description:
** AndroidXrPlatform
*/

#pragma once

#include "evan/openxr/IXrPlatform.hpp"

namespace evan
{
	/**
	 * @brief The AndroidXrPlatform class provides platform-specific
	 * implementations for OpenXR on Android devices.
	 *
	 * This class implements the IXrPlatform interface to provide the necessary
	 * configurations and extensions required for creating an OpenXR instance on
	 * Android. It handles the retrieval of required instance extensions and
	 * provides the appropriate instance creation information for Android
	 * platforms.
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
			 * This is used for handling Android application lifecycle events
			 * and may be necessary for certain OpenXR implementations on
			 * Android.
			 */
			void *androidApp;
		};

		/**
		 * @brief Struct to track the current state of the Android application.
		 * This struct is used to manage the lifecycle of the Android
		 * application and ensure that the OpenXR session responds appropriately
		 * to changes in the application state, such as when the app is resumed,
		 * paused, or stopped.
		 */
		struct AndroidAppState {
#ifdef __ANDROID__
			/**
			 * @brief Pointer to the native Android window.
			 *
			 * This is typically obtained from the Android Native Activity and
			 * is used for rendering and handling input in OpenXR on Android.
			 */
			ANativeWindow *NativeWindow = nullptr;
#endif

			/**
			 * @brief Indicates whether the Android application is currently
			 * resumed. This flag can be used to determine if the OpenXR session
			 * should be active or if it should pause rendering and input
			 * processing when the app is not in the foreground.
			 */
			bool _resumed = false;

			/**
			 * @brief Indicates whether the Android application is currently
			 * paused. This flag can be used to determine if the OpenXR session
			 * should pause rendering and input processing when the app is
			 * paused, such as when the user switches to another app or receives
			 * a phone call.
			 */
			bool _paused = false;
		};

		/**
		 * @brief Constructs an AndroidXrPlatform instance with the specified
		 * Android platform data.
		 *
		 * Initializes the AndroidXrPlatform with the provided Android-specific
		 * data, which is necessary for creating an OpenXR instance on Android
		 * platforms.
		 *
		 * @param platformData The Android-specific platform data required for
		 * OpenXR initialization on Android devices.
		 */
		AndroidXrPlatform(const AndroidPlatformData &platformData);

		/**
		 * @brief Destructor for AndroidXrPlatform.
		 *
		 * Ensures proper cleanup of resources used by the AndroidXrPlatform.
		 */
		~AndroidXrPlatform() override = default;

		/**
		 * @brief Retrieves the list of required OpenXR instance extensions for
		 * Android platform.
		 *
		 * This function returns a vector of extension name strings that must be
		 * enabled when creating an OpenXR instance on Android. The extensions
		 * include Vulkan2 support and Android-specific instance creation
		 * extensions.
		 *
		 * @return std::vector<std::string> A vector containing the required
		 * OpenXR instance extension names for the Android platform, including:
		 *         - XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME: For Vulkan 2 graphics
		 * support
		 *         - XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME: For
		 * Android-specific instance creation (when XR_USE_PLATFORM_ANDROID is
		 * defined)
		 *
		 * @note These extensions should be validated and enabled during OpenXR
		 * instance creation to ensure proper functionality on Android
		 * platforms.
		 *
		 * @see XrInstanceCreateInfo
		 */
		std::vector<std::string> getRequiredInstanceExtensions() const override;

		/**
		 * @brief Retrieves the Android-specific XR instance creation
		 * information.
		 *
		 * This function returns a pointer to the platform-specific instance
		 * creation information structure used for OpenXR initialization on
		 * Android platforms.
		 *
		 * @return A const pointer to the XrBaseInStructure containing Android
		 * XR instance creation parameters when compiled for Android; nullptr
		 * otherwise.
		 *
		 * @note This function is only functional on Android platforms. On other
		 * platforms, it returns nullptr.
		 *
		 * @see _instanceCreateInfoAndroid
		 */
		const XrBaseInStructure *getInstanceCreateInfo() const override;

		/**
		 * @brief Tracks the current state of the Android application.
		 *
		 * This variable is used to keep track of the current state of the
		 * Android application, such as whether it is in the foreground, paused,
		 * or stopped. It can be used to manage OpenXR session state and ensure
		 * that the application responds appropriately to changes in its
		 * lifecycle on Android devices.
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
		 * @brief Stores the Android-specific platform data for OpenXR
		 * initialization.
		 *
		 * This member variable holds the Android-specific data provided to the
		 * constructor, which is necessary for initializing OpenXR on Android
		 * platforms. It may include pointers to the application VM, activity,
		 * and other relevant Android data required for proper OpenXR
		 * functionality on Android devices.
		 */
		AndroidPlatformData _platformData;
	};
}	 // namespace evan