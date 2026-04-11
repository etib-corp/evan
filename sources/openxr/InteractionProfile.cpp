/*
** EPITECH PROJECT, 2026
** evan
** File description:
** InteractionProfile
*/

#include "openxr/InteractionProfile.hpp"

#include <iostream>

evan::InteractionProfile::InteractionProfile()
{
}

std::string evan::InteractionProfile::getRuntimeName(XrInstance instance)
{
	XrInstanceProperties instanceProperties { XR_TYPE_INSTANCE_PROPERTIES };

	XrResult result = xrGetInstanceProperties(instance, &instanceProperties);
	if (result != XR_SUCCESS) {
		std::cerr << "Failed to get OpenXR instance properties: " << result
				  << std::endl;
		return {};
	}
	return std::string(instanceProperties.runtimeName);
}

std::string evan::InteractionProfile::getSystemName(XrInstance instance,
    XrSystemId systemId)
{
	XrSystemProperties systemProperties { XR_TYPE_SYSTEM_PROPERTIES };

	XrResult result = xrGetSystemProperties(instance,
										systemId,
										&systemProperties);
	if (result != XR_SUCCESS) {
		std::cerr << "Failed to get OpenXR system properties: " << result
				  << std::endl;
		return {};
	}
	return std::string(systemProperties.systemName);
}

XrPath evan::InteractionProfile::stringToPath(XrInstance instance,
    const std::string &pathString)
{
	XrPath path = XR_NULL_PATH;

	XrResult result = xrStringToPath(instance,
								 pathString.c_str(), &path);
	if (result != XR_SUCCESS) {
		std::cerr << "Failed to convert string to OpenXR path ('" << pathString
				  << "'): " << result << std::endl;
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
	XrResult result = xrPathToString(instance, path, 0,
									 &pathStringSize, nullptr);
	if (result != XR_SUCCESS || pathStringSize == 0) {
		std::cerr << "Failed to get OpenXR path string size: " << result
				  << std::endl;
		return {};
	}

	std::string pathString(pathStringSize, '\0');
	result = xrPathToString(instance, path, pathStringSize,
						&pathStringSize, pathString.data());
	if (result != XR_SUCCESS) {
		std::cerr << "Failed to convert OpenXR path to string: " << result
				  << std::endl;
		return {};
	}

	if (!pathString.empty() && pathString.back() == '\0') {
		pathString.pop_back();
	}
	return pathString;
}

std::string evan::InteractionProfile::getCurrentInteractionProfilePath(
	XrInstance instance,
    XrSession session,
    const std::string &userPathString)
{
	if (session == XR_NULL_HANDLE) {
		std::cerr << "OpenXR session is not initialized." << std::endl;
		return {};
	}

	XrPath userPath = stringToPath(instance, userPathString);
	if (userPath == XR_NULL_PATH) {
		return {};
	}

	XrInteractionProfileState state { XR_TYPE_INTERACTION_PROFILE_STATE };
	XrResult result =
		xrGetCurrentInteractionProfile(session, userPath, &state);
	if (result != XR_SUCCESS) {
		std::cerr << "Failed to get current interaction profile for '"
				  << userPathString << "': " << result << std::endl;
		return {};
	}

	if (state.interactionProfile == XR_NULL_PATH) {
		return {};
	}
	return pathToString(instance, state.interactionProfile);
}

std::vector<XrPath>
	evan::InteractionProfile::enumerateBoundSourcesForAction(XrInstance instance,
        XrSession session,
        XrAction action)
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
	XrResult result = xrEnumerateBoundSourcesForAction(
		session, &enumerateInfo, 0, &sourceCount, nullptr);
	if (result != XR_SUCCESS || sourceCount == 0) {
		return paths;
	}

	paths.resize(sourceCount, XR_NULL_PATH);
	result = xrEnumerateBoundSourcesForAction(session,
								  &enumerateInfo,
								  sourceCount,
								  &sourceCount,
								  paths.data());
	if (result != XR_SUCCESS) {
		std::cerr << "Failed to enumerate bound action source paths: " << result
				  << std::endl;
		return {};
	}

	return paths;
}

std::vector<std::string> evan::InteractionProfile::
	enumerateBoundSourcePathStringsForAction(
        XrInstance instance,
        XrSession session,
        XrAction action)
{
	std::vector<std::string> sourcePathStrings;
	std::vector<XrPath> sourcePaths = enumerateBoundSourcesForAction(instance,
        session, action);

	sourcePathStrings.reserve(sourcePaths.size());
	for (XrPath sourcePath: sourcePaths) {
		sourcePathStrings.emplace_back(pathToString(instance, sourcePath));
	}
	return sourcePathStrings;
}