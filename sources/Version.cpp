/*
** EPITECH PROJECT, 2025
** maverik
** File description:
** Version
*/

#include "Version.hpp"

maverik::Version::Version(unsigned int major, unsigned int minor,
                          unsigned int patch) {
  this->_major = major;
  this->_minor = minor;
  this->_patch = patch;
}

maverik::Version::~Version() {}

uint32_t maverik::Version::to_uint32_t() {
  return VK_MAKE_VERSION(_major, _minor, _patch);
}
