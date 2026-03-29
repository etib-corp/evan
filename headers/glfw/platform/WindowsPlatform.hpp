/*
** EPITECH PROJECT, 2026
** evan
** File description:
** WindowsPlatform
*/

#pragma once

#include "glfw/IDesktopPlatform.hpp"

namespace evan
{
    /**
     * @brief Windows desktop platform implementation.
     *
     * This class provides an implementation of the IDesktopPlatform interface
     * specifically for the Windows desktop environment. It handles platform-specific
     * functionality related to Windows, such as creating a Vulkan surface using
     * GLFW and managing the necessary resources for rendering on Windows.
     */
    class WindowsDesktopPlatform : public IDesktopPlatform {
        public:
        /**
         * @brief Constructor for WindowsDesktopPlatform.
         * @param name The name of the window.
         * @param width The width of the window.
         * @param height The height of the window.
         * This constructor initializes the Windows desktop platform with the specified
         * window parameters, setting up the necessary resources for rendering on
         * Windows using Vulkan and GLFW.
         */
        WindowsDesktopPlatform(const std::string &name, const uint32_t width,
                               const uint32_t height);

        /**
         * @brief Destructor for WindowsDesktopPlatform.
         * This destructor ensures proper cleanup of resources associated with the
         * Windows desktop platform, such as destroying the Vulkan surface and any
         * other resources that were created during initialization.
         */
        ~WindowsDesktopPlatform() = default;

        /**
         * @brief Get the required instance extensions for the Windows desktop
         * platform.
         *
         * @return A vector of strings representing the required instance
         * extensions for the Windows desktop platform.
         *
         * This method retrieves the necessary Vulkan instance extensions required
         * for the Windows desktop platform, which may include extensions for window
         * surface creation and other platform-specific features related to Vulkan
         * on Windows.
         */
        std::vector<std::string> getRequiredInstanceExtensions() const override;

        /**
         * @brief Create a Vulkan surface for the Windows desktop platform.
         *
         * @param instance The Vulkan instance to use for creating the surface.
         *
         * @return The created Vulkan surface for the Windows desktop platform.
         *
         * This method creates a Vulkan surface using GLFW for the Windows desktop
         * platform. It utilizes GLFW's functionality to create a window surface that
         * can be used with Vulkan for rendering. If surface creation fails, it throws
         * a runtime error indicating that the surface could not be created.
         */
        VkSurfaceKHR createSurface(VkInstance instance) const override;
    };
} // namespace evan