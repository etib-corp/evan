/*
** ETIB PROJECT, 2025
** evan
** File description:
** Error
*/

#pragma once

#include <exception>
#include <string>


namespace evan {
    /**
     * @brief Exception class for handling errors within the evan namespace.
     *
     * This class extends the standard std::exception class to provide
     * custom error handling capabilities specific to the evan project.
     */
    class Error : public std::exception {
        public:

            /**
             * @enum ERROR_CELERITY
             * @brief Specifies discrete levels of error celerity (severity/urgency).
             *
             * This scoped enumeration represents how quickly an error condition should be
             * addressed or how severe its impact is:
             *  - LOW:    Minor issues with low urgency.
             *  - MEDIUM: Moderate issues that warrant attention.
             *  - HIGH:   Severe issues requiring immediate action.
             *
             * Use this enum to classify error conditions and drive logging, alerting,
             * or remediation strategies accordingly.
             */
            enum class ERROR_CELERITY {
                LOW = 1,
                MEDIUM,
                HIGH
            };

            Error(const std::string &message, ERROR_CELERITY level = ERROR_CELERITY::MEDIUM);
            ~Error();

            /**
             * @brief Retrieves the error message associated with the exception.
             *
             * This function overrides the what() method from std::exception to
             * return a custom error message.
             *
             * @return A C-style string representing the error message.
             */
            const char *what() const noexcept override;

            const std::string &getMessage() const { return _message; }

            const ERROR_CELERITY &getLevel() const { return _level; }

        protected:
            std::string _message; // Error message
            ERROR_CELERITY _level; // Error severity level

        private:
    };
} // namespace evan
