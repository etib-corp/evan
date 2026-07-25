/*
** ETIB PROJECT, 2026
** evan
** File description:
** InteractionProfile
*/

#pragma once

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include "evan/EvanPlatform.hpp"
#include <openxr/openxr.h>

#include <string>
#include <vector>
#include <iostream>

namespace evan
{
	/**
	 * @brief A utility class for managing OpenXR interaction profiles,
	 * including retrieving runtime and system information, converting between
	 * strings and paths, and enumerating bound sources for actions.
	 */
	class InteractionProfile:
		protected utility::logging::Loggable<InteractionProfile,
											 utility::logging::DefaultLogger>
	{
		public:
		/**
		 * @brief Deleted default constructor to prevent instantiation of this
		 * utility class.
		 */
		InteractionProfile();

		/**
		 * @brief Retrieves the name of the OpenXR runtime in use.
		 *
		 * @param instance The OpenXR instance handle.
		 * @return The name of the OpenXR runtime, or an empty string on
		 * failure.
		 */
		static std::string getRuntimeName(XrInstance instance);

		/**
		 * @brief Retrieves the name of the OpenXR system.
		 *
		 * @param instance The OpenXR instance handle.
		 * @param systemId The OpenXR system ID.
		 * @return The name of the OpenXR system, or an empty string on failure.
		 */
		static std::string getSystemName(XrInstance instance,
										 XrSystemId systemId);

		/**
		 * @brief Converts a string to an OpenXR path.
		 *
		 * @param instance The OpenXR instance handle.
		 * @param pathString The string representation of the path.
		 * @return The corresponding OpenXR path, or XR_NULL_PATH on failure.
		 */
		static XrPath stringToPath(XrInstance instance,
								   const std::string &pathString);

		/**
		 * @brief Converts an OpenXR path to a string.
		 * @param instance The OpenXR instance handle.
		 * @param path The OpenXR path to convert.
		 * @return The string representation of the OpenXR path, or an empty
		 * string on failure.
		 */
		static std::string pathToString(XrInstance instance, XrPath path);

		/**
		 * @brief Retrieves the current interaction profile path for a given
		 * user path.
		 *
		 * @param instance The OpenXR instance handle.
		 * @param session The OpenXR session handle.
		 * @param userPathString The string representation of the user path
		 * (e.g., "/user/hand/left").
		 * @return The string representation of the current interaction profile
		 * path, or an empty string on failure.
		 */
		static std::string
			getCurrentInteractionProfilePath(XrInstance instance,
											 XrSession session,
											 const std::string &userPathString);

		/**
		 * @brief Enumerates the bound source paths for a given action.
		 *
		 * @param instance The OpenXR instance handle.
		 * @param session The OpenXR session handle.
		 * @param action The OpenXR action handle.
		 * @return A vector of OpenXR paths representing the bound sources for
		 * the action.
		 */
		static std::vector<XrPath>
			enumerateBoundSourcesForAction(XrInstance instance,
										   XrSession session, XrAction action);

		/**
		 * @brief Enumerates the bound source path strings for a given action.
		 *
		 * @param instance The OpenXR instance handle.
		 * @param session The OpenXR session handle.
		 * @param action The OpenXR action handle.
		 * @return A vector of strings representing the bound source paths for
		 * the action.
		 */
		static std::vector<std::string>
			enumerateBoundSourcePathStringsForAction(XrInstance instance,
													 XrSession session,
													 XrAction action);
	};
}	 // namespace evan