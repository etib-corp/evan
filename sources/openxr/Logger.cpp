/*
** ETIB PROJECT, 2025
** evan
** File description:
** Logger
*/

#include "openxr/Logger.hpp"
#include <android/log.h>

evan::openxr::Logger::Logger(const std::string &programName,
                              const evan::ALogger::Environment &env)
{
  _env = env == evan::ALogger::DEV ? "DEV    " : "PROD   ";
  _programName = programName + "    ";
}

void evan::openxr::Logger::log(const std::string &message,
                             const std::string &logLevel,
                             const std::string &caller) const {
  std::string out = "";
  std::stringstream ss;
  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);

  out += _env;
  out += _programName;
  out += logLevel;
  ss << std::put_time(&tm, "%b-%d %H:%M:%S    ");
  out += ss.str();
  out += caller + "    ";
  // out += std::to_string(_errorCode);
  out += message + "    ";
  __android_log_print(ANDROID_LOG_INFO, "evan_openxr", "%s\n", out.c_str());
}

evan::ALogger::LogLevel evan::openxr::Logger::getLogLevelFromString(const std::string &logLevel) const
{
  if (logLevel == "\033[39;41mFATAL\033[39;49m     ") {
    return evan::ALogger::FATAL;
  } else if (logLevel == "\033[31mERROR\033[39m     ") {
    return evan::ALogger::ERROR;
  } else if (logLevel == "\033[33mWARNING\033[39m   ") {
    return evan::ALogger::WARNING;
  } else if (logLevel == "\033[36mINFO\033[39m      ") {
    return evan::ALogger::INFO;
  } else if (logLevel == "\033[32mDEBUG\033[39m     ") {
    return evan::ALogger::DEBUG;
  } else {
    return evan::ALogger::INFO; // default
  }
}
