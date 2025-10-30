/*
** ETIB PROJECT, 2025
** evan
** File description:
** openxr-include
*/

#pragma once

// Platform-specific defines for OpenXR
#if defined(__ANDROID__)
#define XR_USE_PLATFORM_ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__APPLE__)
// macOS doesn't have a specific OpenXR platform define
// We'll use headless or rely on the loader to detect the platform
#elif defined(_WIN32)
#define XR_USE_PLATFORM_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__)
#define XR_USE_PLATFORM_XLIB
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#define XR_USE_GRAPHICS_API_VULKAN

#include "Evan.hpp"

#include <openxr/openxr.h>
#if defined(__ANDROID__)
#include <jni.h>
#endif
#include <openxr/openxr_platform.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <map>
#include <memory>
