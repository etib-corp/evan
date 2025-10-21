/*
** ETIB PROJECT, 2025
** evan
** File description:
** Logger
*/

#pragma once

#include "ALogger.hpp"

/**
 * @namespace evan
 */
namespace evan {
/**
 * @namespace vk
 */
namespace vk {
/**
 * @class Logger
 * @brief The `evan::vk::Logger` class is a concrete implementation of the
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
   * @param stream The `std::ostream` reference to the output stream where log
   * messages will be written (e.g., `std::cout`, `std::cerr`).
   * @param programName The name of the program using the logger.
   * @param env The environment in which the logger operates (e.g., `DEV` or
   * `PROD`). Default is `DEV`.
   */
  Logger(std::ostream &stream, const std::string &programName,
         const Environment &env = DEV);

  /**
   * @brief Destroy the Logger object
   */
  ~Logger() override = default;

  /**
   * @brief The log method in the `evan::vk::Logger` class is a virtual
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
  std::ostream &_stream; ///< A reference to the output stream where log
                         ///< messages will be written
};
}; // namespace vk
} // namespace evan