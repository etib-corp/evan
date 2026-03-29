/*
** EPITECH PROJECT, 2026
** evan
** File description:
** LinuxXrPlatform
*/

#pragma once

#include "openxr/IXrPlatform.hpp"

namespace evan {

    /**
     * @brief The LinuxXrPlatform class provides platform-specific implementations
     * for OpenXR on Linux devices.
     *
     * This class implements the IXrPlatform interface to provide the necessary
     * configurations and extensions required for creating an OpenXR instance on
     * Linux. It handles the retrieval of required instance extensions and
     * provides the appropriate instance creation information for Linux platforms.
     */
    class LinuxXrPlatform: public IXrPlatform
    {
        public:
        /**
         * @brief Constructs a LinuxXrPlatform instance.
         *
         * Initializes the LinuxXrPlatform with default settings.
         * This constructor does not take any parameters and sets up the platform for use on Linux.
         */
        LinuxXrPlatform();

        /**
         * @brief Destructor for the LinuxXrPlatform class.
         *
         * This destructor is responsible for cleaning up any resources allocated
         * by the LinuxXrPlatform instance. It ensures that any platform-specific
         * resources are properly released when the instance is destroyed.
         */
        ~LinuxXrPlatform() = default;

        public:
        /**
         * @brief Retrieves the required instance extensions for the Linux OpenXR platform.
         *
         * This function returns a vector of strings representing the required
         * instance extensions for the Linux OpenXR platform. These extensions are
         * necessary for proper functionality and compatibility with the OpenXR
         * runtime on Linux.
         *
         * @return A vector of strings containing the required instance
         * extensions for the Linux OpenXR platform.
         */
        std::vector<std::string> getRequiredInstanceExtensions() const override;

        /**
         * @brief Retrieves the instance creation information for the Linux OpenXR platform.
         *
         * This function returns a pointer to the instance creation information
         * specific to the Linux OpenXR platform. This information is typically
         * used when creating an OpenXR instance on Linux.
         *
         * @return A pointer to the instance creation information for the Linux
         * OpenXR platform as an XrBaseInStructure.
         */
        const XrBaseInStructure *getInstanceCreateInfo() const override;
    };

} // namespace evan