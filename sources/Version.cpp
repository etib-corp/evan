/*
** ETIB PROJECT, 2025
** evan
** File description:
** Version
*/

#include "evan/Version.hpp"

evan::Version::Version(unsigned int major, unsigned int minor,
					   unsigned int patch)
{
	this->getLogger().info() << "Constructing Version object with version "
							 << major << "." << minor << "." << patch;

	this->_major = major;
	this->_minor = minor;
	this->_patch = patch;
}

evan::Version::~Version()
{
	this->getLogger().info() << "Destroying Version object...";
}

uint32_t evan::Version::to_uint32_t()
{
	return VK_MAKE_VERSION(_major, _minor, _patch);
}
