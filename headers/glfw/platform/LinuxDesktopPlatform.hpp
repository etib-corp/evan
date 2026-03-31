/*
** ETIB PROJECT, 2026
** evan
** File description:
** LinuxDesktopPlatform
*/

#pragma once

#include "glfw/IDesktopPlatform.hpp"

namespace evan {
    /**
     * @brief Linux desktop platform implementation.
     *
     * This class provides an implementation of the IDesktopPlatform interface
     * specifically for the Linux desktop environment. It handles platform-specific
     * functionality related to Linux, such as creating a Vulkan surface using
     * GLFW and managing the necessary resources for rendering on Linux.
     */
    class LinuxDesktopPlatform : public IDesktopPlatform {
        public:
        /**
         * @brief Constructor for LinuxDesktopPlatform.
         * @param name The name of the window.
         * @param width The width of the window.
         * @param height The height of the window.
         * This constructor initializes the Linux desktop platform with the specified
         * window parameters, setting up the necessary resources for rendering on
         * Linux using Vulkan and GLFW.
         */
        LinuxDesktopPlatform(const std::string &name, const uint32_t width,
                            const uint32_t height);

        /**
         * @brief Destructor for LinuxDesktopPlatform.
         * This destructor ensures proper cleanup of resources associated with the
         * Linux desktop platform, such as destroying the Vulkan surface and any
         * other resources that were created during initialization.
         */
        ~LinuxDesktopPlatform() = default;

        /**
         * @brief Get the required instance extensions for the Linux desktop
         * platform.
         *
         * @return A vector of strings representing the required instance
         * extensions for the Linux desktop platform.
         *
         * This method retrieves the necessary Vulkan instance extensions required
         * for the Linux desktop platform, which may include extensions for window
         * surface creation and other platform-specific features related to Vulkan
         * on Linux.
         */
        std::vector<std::string> getRequiredInstanceExtensions() const override;

        /**
         * @brief Create a Vulkan surface for the Linux desktop platform.
         * @param instance The Vulkan instance to use for creating the surface.
         * @return The created Vulkan surface.
         * This method creates a Vulkan surface that is compatible with the Linux
         * desktop platform, allowing rendered images to be presented on the screen. The implementation may involve
         * using GLFW's functions to create a window surface that can be used with Vulkan on Linux.
         */
        VkSurfaceKHR createSurface(VkInstance instance) const override;
    };
} // namespace evan