/*
** ETIB PROJECT, 2025
** evan
** File description:
** vulkan
*/

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#ifdef __GLFW__

#ifdef _WIN32
// Prevent Windows.h min/max macros from interfering with std::min/std::max
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <GLFW/glfw3native.h>

#endif

#ifdef __OPENXR__

#include <openxr/openxr.h>
#include <vulkan/vulkan.h>

#endif
