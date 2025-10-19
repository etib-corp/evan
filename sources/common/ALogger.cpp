/*
** ETIB PROJECT, 2025
** maverik
** File description:
** ALogger
*/

#include "ALogger.hpp"

maverik::ALogger::~ALogger() = default;

void maverik::ALogger::fatal(const std::string &message,
                             const std::string &caller) const {
  this->log(message, "\033[39;41mFATAL\033[39;49m     ", caller);
}

void maverik::ALogger::error(const std::string &message,
                             const std::string &caller) const {
  this->log(message, "\033[31mERROR\033[39m     ", caller);
}

void maverik::ALogger::warning(const std::string &message,
                               const std::string &caller) const {
  this->log(message, "\033[33mWARNING\033[39m   ", caller);
}

void maverik::ALogger::info(const std::string &message,
                            const std::string &caller) const {
  this->log(message, "\033[36mINFO\033[39m      ", caller);
}

void maverik::ALogger::debug(const std::string &message,
                             const std::string &caller) const {
  this->log(message, "\033[32mDEBUG\033[39m     ", caller);
}
