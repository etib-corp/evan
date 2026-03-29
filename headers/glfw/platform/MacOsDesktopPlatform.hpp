/*
** EPITECH PROJECT, 2026
** evan
** File description:
** MacOsDesktopPlatform
*/

#pragma once

#include "glfw/IDesktopPlatform.hpp"

namespace evan {
    /**
     * @brief MacOS desktop platform implementation.
     *
     * This class provides an implementation of the IDesktopPlatform interface
     * specifically for the MacOS desktop environment. It handles platform-specific
     * functionality related to MacOS, such as creating a Vulkan surface using
     * MoltenVK and managing the necessary resources for rendering on MacOS.
     */
    class MacOsDesktopPlatform : public IDesktopPlatform {
        public:
        /**
         * @brief Constructor for MacOsDesktopPlatform.
         * @param name The name of the window.
         * @param width The width of the window.
         * @param height The height of the window.
         * This constructor initializes the MacOS desktop platform with the specified
         * window parameters, setting up the
         * necessary resources for rendering on MacOS using Vulkan and MoltenVK.
         */
        MacOsDesktopPlatform(const std::string &name, const uint32_t width,
                            const uint32_t height);

        /**
         * @brief Destructor for MacOsDesktopPlatform.
         * This destructor ensures proper cleanup of resources associated with the
         * MacOS desktop platform, such as destroying the Vulkan surface and any
         * other resources that were created during initialization.
         */
        ~MacOsDesktopPlatform() = default;

        /**
         * @brief Get the required instance extensions for the MacOS desktop
         * platform.
         *
         * @return A vector of strings representing the required instance
         * extensions for the MacOS desktop platform.
         *
         * This method retrieves the necessary Vulkan instance extensions required
         * for the MacOS desktop platform, which may include extensions for
         * window surface creation and other platform-specific features related to
         * MoltenVK and Vulkan on MacOS.
         */
        std::vector<std::string> getRequiredInstanceExtensions() const override;

        /**
         * @brief Create a Vulkan surface for the MacOS desktop platform.
         *
         * @param instance The Vulkan instance to use for creating the surface.
         * @return The created Vulkan surface handle.
         *
         * This method creates a Vulkan surface that is compatible with the MacOS
         * desktop platform, allowing rendered images to be presented on the
         * screen. The implementation may involve using MoltenVK's functions to
         * create a window surface that can be used with Vulkan on MacOS.
         */
        VkSurfaceKHR createSurface(VkInstance instance) const override;
    };
} // namespace evan