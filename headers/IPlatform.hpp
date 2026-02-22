/*
** EPITECH PROJECT, 2026
** evan
** File description:
** IPlatform
*/

#pragma once

#include "EvanPlatform.hpp"

#include <string>

namespace evan {
    /**
     * @brief Abstract interface for platform-specific functionality.
     * 
     * Defines a common interface for platform implementations. Derived classes
     * must implement platform-specific behavior.
     */
    class IPlatform {
        public:
            /**
             * @brief Virtual destructor.
             * 
             * Ensures proper cleanup of derived class resources.
             */
            virtual ~IPlatform() = default;
            
            /**
             * @brief Get the required instance extensions for the platform.
             * 
             * @return A vector of strings representing the required instance extensions.
             */
            virtual std::vector<std::string> getRequiredInstanceExtensions() const = 0;

            /**
             * @brief Check if the platform should close.
             * 
             * @return true if the platform should close, false otherwise.
             */
            virtual bool shouldClose() const = 0;

            /**
             * @brief Poll for platform events.
             * 
             * This method should be called regularly to process platform events.
             */
            virtual void pollEvents() = 0;

        protected:

    };
} // namespace evan