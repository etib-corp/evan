/*
** EPITECH PROJECT, 2025
** evan
** File description:
** Version
*/

#ifndef VERSION_HPP_
#define VERSION_HPP_

#include "EvanPlatform.hpp"

namespace evan
{
	/**
	 * @class Version
	 * @brief Represents a software version using major, minor, and patch
	 * numbers.
	 *
	 * The Version class encapsulates version information and provides a method
	 * to convert the version to a 32-bit unsigned integer representation.
	 *
	 * @var Version::_major
	 *      The major version number.
	 * @var Version::_minor
	 *      The minor version number.
	 * @var Version::_patch
	 *      The patch version number.
	 */
	class Version
	{
		public:
		/**
		 * @brief Constructs a Version object with the specified major, minor,
		 * and patch numbers.
		 *
		 * @param major The major version number.
		 * @param minor The minor version number.
		 * @param patch The patch version number.
		 */
		Version(unsigned int major, unsigned int minor, unsigned int patch);

		/**
		 * @brief Destructor for the Version class.
		 *
		 * Cleans up any resources used by the Version instance.
		 * Currently, this destructor does not perform any specific actions.
		 */
		~Version();

		/**
		 * @brief Converts the version information to a 32-bit unsigned integer.
		 *
		 * This function encodes the major, minor, and patch version numbers
		 * into a single 32-bit unsigned integer using the VK_MAKE_VERSION
		 * macro, which is commonly used for Vulkan versioning.
		 *
		 * @return uint32_t The encoded version as a 32-bit unsigned integer.
		 */
		uint32_t to_uint32_t();

		unsigned int _major;	// Major version number
		unsigned int _minor;	// Minor version number
		unsigned int _patch;	// Patch version number
	};
}	 // namespace evan

#endif /* !VERSION_HPP_ */
