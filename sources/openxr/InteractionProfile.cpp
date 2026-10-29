/*
** ETIB PROJECT, 2026
** evan
** File description:
** InteractionProfile
*/

#include "evan/openxr/InteractionProfile.hpp"

////////////////////
// Public Methods //
////////////////////

evan::InteractionProfile::InteractionProfile()
	: utility::logging::Loggable<InteractionProfile,
								 utility::logging::DefaultLogger>()
{
}

std::string evan::InteractionProfile::getRuntimeName(XrInstance instance)
{
	XrInstanceProperties instanceProperties { XR_TYPE_INSTANCE_PROPERTIES };

	XrResult result = xrGetInstanceProperties(instance, &instanceProperties);
	if (result != XR_SUCCESS) {
		return {};
	}
	return std::string(instanceProperties.runtimeName);
}

std::string evan::InteractionProfile::getSystemName(XrInstance instance,
													XrSystemId systemId)
{
	XrSystemProperties systemProperties { XR_TYPE_SYSTEM_PROPERTIES };

	XrResult result =
		xrGetSystemProperties(instance, systemId, &systemProperties);
	if (result != XR_SUCCESS) {
		return {};
	}
	return std::string(systemProperties.systemName);
}

XrPath evan::InteractionProfile::stringToPath(XrInstance instance,
											  const std::string &pathString)
{
	XrPath path = XR_NULL_PATH;

	XrResult result = xrStringToPath(instance, pathString.c_str(), &path);
	if (result != XR_SUCCESS) {
		return XR_NULL_PATH;
	}
	return path;
}

std::string evan::InteractionProfile::pathToString(XrInstance instance,
												   XrPath path)
{
	if (path == XR_NULL_PATH) {
		return {};
	}

	uint32_t pathStringSize = 0;
	XrResult result =
		xrPathToString(instance, path, 0, &pathStringSize, nullptr);
	if (result != XR_SUCCESS || pathStringSize == 0) {
		return {};
	}

	std::string pathString(pathStringSize, '\0');
	result = xrPathToString(instance, path, pathStringSize, &pathStringSize,
							pathString.data());
	if (result != XR_SUCCESS) {
		return {};
	}

	if (!pathString.empty() && pathString.back() == '\0') {
		pathString.pop_back();
	}
	return pathString;
}

std::string evan::InteractionProfile::getCurrentInteractionProfilePath(
	XrInstance instance, XrSession session, const std::string &userPathString)
{
	if (session == XR_NULL_HANDLE) {
		return {};
	}

	XrPath userPath = stringToPath(instance, userPathString);
	if (userPath == XR_NULL_PATH) {
		return {};
	}

	XrInteractionProfileState state { XR_TYPE_INTERACTION_PROFILE_STATE };
	XrResult result = xrGetCurrentInteractionProfile(session, userPath, &state);
	if (result != XR_SUCCESS) {
		return {};
	}

	if (state.interactionProfile == XR_NULL_PATH) {
		return {};
	}
	return pathToString(instance, state.interactionProfile);
}

std::vector<XrPath> evan::InteractionProfile::enumerateBoundSourcesForAction(
	XrInstance instance, XrSession session, XrAction action)
{
	std::vector<XrPath> paths;

	if (session == XR_NULL_HANDLE || action == XR_NULL_HANDLE) {
		return paths;
	}

	XrBoundSourcesForActionEnumerateInfo enumerateInfo {
		XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO
	};
	enumerateInfo.action = action;

	uint32_t sourceCount = 0;
	XrResult result		 = xrEnumerateBoundSourcesForAction(
		 session, &enumerateInfo, 0, &sourceCount, nullptr);
	if (result != XR_SUCCESS || sourceCount == 0) {
		return paths;
	}

	paths.resize(sourceCount, XR_NULL_PATH);
	result = xrEnumerateBoundSourcesForAction(
		session, &enumerateInfo, sourceCount, &sourceCount, paths.data());
	if (result != XR_SUCCESS) {
		return {};
	}

	return paths;
}

std::vector<std::string>
	evan::InteractionProfile::enumerateBoundSourcePathStringsForAction(
		XrInstance instance, XrSession session, XrAction action)
{
	std::vector<std::string> sourcePathStrings;
	std::vector<XrPath> sourcePaths =
		enumerateBoundSourcesForAction(instance, session, action);

	sourcePathStrings.reserve(sourcePaths.size());
	for (XrPath sourcePath: sourcePaths) {
		sourcePathStrings.emplace_back(pathToString(instance, sourcePath));
	}
	return sourcePathStrings;
}

const std::vector<std::string> &
	evan::InteractionProfile::getDefaultProfileCandidates()
{
	static const std::vector<std::string> candidates {
		"/interaction_profiles/oculus/touch_controller",
		"/interaction_profiles/microsoft/mixed_reality/motion_controller",
		"/interaction_profiles/hp/mixed_reality/motion_controller",
		"/interaction_profiles/valve/index_controller",
		"/interaction_profiles/khr/simple_controller",
	};
	return candidates;
}

std::string evan::InteractionProfile::getPreferredInteractionProfilePath(
	XrInstance instance, const std::vector<std::string> &candidateProfiles,
	const std::string &fallbackProfile)
{
	if (instance == XR_NULL_HANDLE) {
		return fallbackProfile;
	}

	for (const auto &candidateProfile: candidateProfiles) {
		if (isInteractionProfileSupported(instance, candidateProfile)) {
			return candidateProfile;
		}
	}
	return fallbackProfile;
}

bool evan::InteractionProfile::isInteractionProfileSupported(
	XrInstance instance, const std::string &profilePath)
{
	if (instance == XR_NULL_HANDLE) {
		return false;
	}

	XrPath interactionProfile = stringToPath(instance, profilePath);
	if (interactionProfile == XR_NULL_PATH) {
		return false;
	}

	XrInteractionProfileSuggestedBinding suggestedBindings {
		XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING
	};
	suggestedBindings.interactionProfile	 = interactionProfile;
	suggestedBindings.countSuggestedBindings = 0;
	suggestedBindings.suggestedBindings		 = nullptr;

	XrResult result =
		xrSuggestInteractionProfileBindings(instance, &suggestedBindings);
	return result == XR_SUCCESS;
}