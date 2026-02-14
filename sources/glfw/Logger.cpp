/*
** ETIB PROJECT, 2025
** evan
** File description:
** Logger
*/

#include "glfw/Logger.hpp"

evan::glfw::Logger::Logger(std::ostream &stream, const std::string &programName,
						   const evan::ALogger::Environment &env)
	: _stream(stream)
{
	_env		 = env == evan::ALogger::DEV ? "DEV    " : "PROD   ";
	_programName = programName + "    ";
}

void evan::glfw::Logger::log(const std::string &message,
							 const std::string &logLevel,
							 const std::string &caller) const
{
	std::string out = "";
	std::stringstream ss;
	std::time_t t = std::time(nullptr);
	std::tm tm	  = *std::localtime(&t);

	out += _env;
	out += _programName;
	out += logLevel;
	ss << std::put_time(&tm, "%b-%d %H:%M:%S    ");
	out += ss.str();
	out += caller + "    ";
	// out += std::to_string(_errorCode);
	out += message + "    ";
	out += "\n";
	_stream << out;
	_stream.flush();
}
