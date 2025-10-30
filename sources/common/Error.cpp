/*
** ETIB PROJECT, 2025
** evan
** File description:
** Error
*/

#include "Error.hpp"

evan::Error::Error(const std::string &message) : _message(message)
{
}

evan::Error::~Error()
{
}

const char *evan::Error::what() const noexcept
{
    return _message.c_str();
}
