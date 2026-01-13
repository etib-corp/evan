/*
 Copyright (c) 2025 ETIB Corporation

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "ALogger.hpp"

/**
 * @namespace evan
 */
namespace evan {
/**
 * @namespace openxr
 */
namespace openxr {
/**
 * @class Logger
 * @brief The `evan::openxr::Logger` class is a concrete implementation of the
 * `evan::ALogger` interface, designed to log messages to a specified output
 * stream. It provides functionality to log messages with a log level and caller
 * information, and it supports initialization with a program name and an
 * optional environment setting.
 */
class Logger : public evan::ALogger {
public:
  /**
   * @brief Construct a new Logger object
   *
   * @param programName The name of the program using the logger.
   * @param env The environment in which the logger operates (e.g., `DEV` or
   * `PROD`). Default is `DEV`.
   */
  Logger(const std::string &programName, const Environment &env = DEV);

  /**
   * @brief Destroy the Logger object
   */
  ~Logger() override = default;

  /**
   * @brief The log method in the `evan::openxr::Logger` class is a virtual
   * function that overrides a base class method. It logs a message with a
   * specified log level and caller information, taking three constant string
   * references as parameters and ensuring no modification to the class state
   * due to its const qualifier.
   * @param message The message to log.
   * @param logLevel The log level (e.g., ERROR, WARNING, INFO, DEBUG).
   * @param caller The name of the function or method that called this function.
   */
  void log(const std::string &message, const std::string &logLevel,
           const std::string &caller) const override;
private:
  LogLevel getLogLevelFromString(const std::string &logLevel) const;
};
}; // namespace openxr
} // namespace evan