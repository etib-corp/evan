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
            Error(const std::string &message);
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

        protected:
            std::string _message; // Error message

        private:
    };
} // namespace evan
