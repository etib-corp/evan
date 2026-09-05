/*
** ETIB PROJECT, 2026
** evan
** File description:
** CheckedCast
*/

#pragma once

#include <stdexcept>
#include <string>

namespace evan
{
	/**
	 * @brief Performs a checked dynamic cast and throws on failure.
	 *
	 * Unlike a raw dynamic_cast, which silently returns null (or throws
	 * std::bad_cast for references), this helper turns a failed downcast into
	 * an explicit std::logic_error so backend type mismatches fail loudly
	 * instead of being dereferenced.
	 *
	 * @tparam Target The type to cast to.
	 * @tparam Source The source object type.
	 * @param source The object to cast.
	 * @return Target& A reference to the casted object.
	 * @throws std::logic_error If the cast fails.
	 */
	template <typename Target, typename Source>
	Target &checkedCast(Source &source)
	{
		Target *result = dynamic_cast<Target *>(&source);
		if (result == nullptr) {
			throw std::logic_error("Failed to cast to requested backend type");
		}
		return *result;
	}
}	 // namespace evan
