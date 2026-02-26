/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrPlatform
*/

#pragma once

#include "IPlatform.hpp"

#include <iostream>

namespace evan {
    /**
     * @brief Desktop platform implementation.
     *
     * This class provides an implementation of the IPlatform interface for
     * Desktop. It handles platform-specific functionality related to Desktop.
     */
    class DesktopPlatform : public IPlatform {
        public:
            /**
             * @brief Constructor for DesktopPlatform.
             *
             * @param name The name of the window.
             * @param width The width of the window.
             * @param height The height of the window.
             */
            DesktopPlatform(const std::string &name, const uint32_t width, const uint32_t height);

            /**
             * @brief Destructor for DesktopPlatform.
             *
             * Ensures proper cleanup of Desktop resources.
             */
            ~DesktopPlatform() override;

            std::vector<std::string> getRequiredInstanceExtensions() const override;

            bool shouldClose() const override;

            void pollEvents() override;

        protected:
            GLFWwindow *_window = nullptr; // Pointer to the GLFW window
    };
} // namespace evan