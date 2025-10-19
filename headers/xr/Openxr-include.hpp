/*
** ETIB PROJECT, 2025
** maverik
** File description:
** openxr-include
*/

#pragma once

#define XR_USE_PLATFORM_ANDROID
#define XR_USE_GRAPHICS_API_VULKAN
#define VK_USE_PLATFORM_ANDROID_KHR

#include "maverik.hpp"

#include <jni.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <map>
#include <memory>
