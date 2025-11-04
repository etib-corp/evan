/*
** ETIB PROJECT, 2025
** evan
** File description:
** Error
*/

#include "Error.hpp"

evan::Error::Error(const std::string &message, ERROR_CELERITY level) : _message(message), _level(level)
{
}

evan::Error::~Error()
{
}

const char *evan::Error::what() const noexcept
{
    std::string fullMessage = _message + " (Error Celerity: ";
    switch (_level) {
        case ERROR_CELERITY::LOW:
            fullMessage += "LOW";
            break;
        case ERROR_CELERITY::MEDIUM:
            fullMessage += "MEDIUM";
            break;
        case ERROR_CELERITY::HIGH:
            fullMessage += "HIGH";
            break;
    }
    fullMessage += ")";
    return fullMessage.c_str();
}
