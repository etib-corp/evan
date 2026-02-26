/*
 Copyright (c) 2025 ETIB Corporation

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#pragma once

// GLM configuration
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

/*
 * @brief Validation layers for Vulkan debugging.
 *
 * This constant defines the validation layers used for debugging Vulkan
 * applications. These layers provide additional checks and validation
 * during development to help identify issues and improve code quality.
 *
 */
const std::vector<const char *> validationLayers = {
	"VK_LAYER_KHRONOS_validation"	 // Khronos validation layer
};

/*
 ** @brief Callback type for the debug messenger.
 **
 ** This callback is used to handle debug messages from the Vulkan
 * API.
 ** It is used to log the debug messages from the Vulkan API.
 **
 ** @param severity the severity of the message
 ** @param type the type of the message
 ** @param pCallbackData the callback data
 ** @param pUserData the user data
 **
 ** @return true if the message is handled, false otherwise
 */
typedef VkBool32 debugCallback_t(VkDebugUtilsMessageSeverityFlagBitsEXT,
								 VkDebugUtilsMessageTypeFlagsEXT,
								 const VkDebugUtilsMessengerCallbackDataEXT *,
								 void *);

// Vulkan headers - needed for both GLFW and OpenXR
#if defined(__GLFW__) || defined(__OPENXR__)
	#include <vulkan/vulkan.h>
#endif

#include <vulkan/vulkan.h>

#ifdef __GLFW__
	#include <vulkan/vulkan.hpp>
	#define GLFW_INCLUDE_VULKAN
	#include <GLFW/glfw3.h>
	#include <GLFW/glfw3native.h>
#endif

#ifdef __OPENXR__
	#ifdef _WIN32
		#include <windows.h>
	#endif
	#include <openxr/openxr.h>
	#ifdef __ANDROID__
		#include <jni.h>
	#endif
	#include <openxr/openxr_platform.h>
#endif
