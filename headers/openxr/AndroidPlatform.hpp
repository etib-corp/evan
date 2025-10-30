/*
** ETIB PROJECT, 2025
** evan
** File description:
** AndroidPlatform
*/

#pragma once

#include "EvanPlatform.hpp"
#include <memory>

// Platform data structure - defined for all platforms but only populated on
// Android
struct PlatformData {
  void *applicationVM;
  void *applicationActivity;
};

#ifdef __ANDROID__
namespace evan {
namespace openxr {
/**
 * @class AndroidPlatform
 *
 * @brief A class to handle Android-specific OpenXR platform initialization and
 * instance creation.
 *
 * This class is responsible for initializing the OpenXR loader for Android and
 * providing the necessary Android-specific instance creation information. It
 * utilizes the `PlatformData` structure to obtain the Android application VM
 * and activity required for OpenXR initialization.
 */
class AndroidPlatform {
public:
  /**
   * @brief Constructs an AndroidPlatform object and initializes the OpenXR
   * loader for Android.
   *
   * This constructor attempts to retrieve and call the `xrInitializeLoaderKHR`
   * function to initialize the OpenXR loader with Android-specific information
   * provided in the `PlatformData` structure. It sets up the loader
   * initialization info and stores the instance creation info for later use.
   *
   * @param platformData Shared pointer to a PlatformData structure containing
   * Android application VM and activity.
   */
  AndroidPlatform(std::shared_ptr<PlatformData> platformData);

  // Destructor
  ~AndroidPlatform();

  /**
   * @brief Retrieves the Android-specific instance creation information.
   *
   * This function returns a pointer to the Android-specific instance creation
   * structure, cast to a generic XrBaseInStructure pointer. This is typically
   * used when creating an OpenXR instance on an Android platform.
   *
   * @return A pointer to the Android-specific instance creation information
   *         as an XrBaseInStructure.
   */
  XrBaseInStructure *getInstanceCreateInfoAndroid() {
    return reinterpret_cast<XrBaseInStructure *>(&_instanceCreateInfoAndroid);
  }

protected:
  XrInstanceCreateInfoAndroidKHR
      _instanceCreateInfoAndroid{}; // Android-specific instance creation
                                    // information
private:
};

} // namespace openxr
} // namespace evan
#endif // __ANDROID__
