/*
** EPITECH PROJECT, 2025
** evan
** File description:
** Software
*/

#include "glfw/Software.hpp"

/*
** Construct a new Software object.
**
** This constructor is used to create a new Software object.
** It initializes the application name, application version, engine name and
* engine version.
**
** The default values are:
** - Application name `_appName`: "Hello, World !"
** - Application version `_appVersion`: 1.0.0
** - Engine name `_engineName`: "Evan"
** - Engine version `_engine`: 1.0.0
*/
evan::glfw::Software::Software()
{
	this->_appName			= "Hello, World !";
	this->_appVersion		= new Version(1, 0, 0);
	this->_engineName		= "Evan";
	this->_engineVersion	= new Version(1, 0, 0);
	this->_graphicalContext = std::make_shared<evan::glfw::GraphicalContext>();
}

/*
** Construct a new Software object.
**
** This constructor is used to create a new Software object.
** It initializes the application name, application version, engine name and
* engine version.
**
** @param appName the application name
** @param appVersion the application version
** @param engineName the engine name
** @param engineVersion the engine version
**
*/
evan::glfw::Software::Software(const std::string &appName,
							   const Version &appVersion,
							   const std::string &engineName,
							   const Version &engineVersion)
{
	this->_appName			= appName;
	this->_appVersion		= new Version(appVersion);
	this->_engineName		= engineName;
	this->_engineVersion	= new Version(engineVersion);
	this->_graphicalContext = std::make_shared<evan::glfw::GraphicalContext>(
		appName, appVersion, engineName, engineVersion);
}

evan::glfw::Software::~Software()
{
}
