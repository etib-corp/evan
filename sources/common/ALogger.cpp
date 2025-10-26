/*
** ETIB PROJECT, 2025
** evan
** File description:
** ALogger
*/

#include "ALogger.hpp"

evan::ALogger::~ALogger() = default;

void evan::ALogger::fatal(const std::string &message,
                          const std::string &caller) const {
  this->log(message, "\033[39;41mFATAL\033[39;49m     ", caller);
}

void evan::ALogger::error(const std::string &message,
                          const std::string &caller) const {
  this->log(message, "\033[31mERROR\033[39m     ", caller);
}

void evan::ALogger::warning(const std::string &message,
                            const std::string &caller) const {
  this->log(message, "\033[33mWARNING\033[39m   ", caller);
}

void evan::ALogger::info(const std::string &message,
                         const std::string &caller) const {
  this->log(message, "\033[36mINFO\033[39m      ", caller);
}

void evan::ALogger::debug(const std::string &message,
                          const std::string &caller) const {
  this->log(message, "\033[32mDEBUG\033[39m     ", caller);
}
