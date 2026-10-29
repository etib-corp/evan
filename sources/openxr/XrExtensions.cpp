/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrExtensions
*/

#include "evan/openxr/XrExtensions.hpp"

#include <algorithm>
#include <array>

std::vector<evan::XrExtensionInfo>
	evan::XrExtensions::enumerateInstanceExtensions(XrResult *outResult)
{
	std::uint32_t count = 0;
	XrResult result =
		xrEnumerateInstanceExtensionProperties(nullptr, 0, &count, nullptr);
	if (outResult != nullptr) {
		*outResult = result;
	}
	if (result != XR_SUCCESS) {
		return {};
	}

	std::vector<XrExtensionProperties> properties(
		count, XrExtensionProperties { XR_TYPE_EXTENSION_PROPERTIES });
	result = xrEnumerateInstanceExtensionProperties(nullptr, count, &count,
													properties.data());
	if (result == XR_ERROR_SIZE_INSUFFICIENT) {
		properties.assign(
			count, XrExtensionProperties { XR_TYPE_EXTENSION_PROPERTIES });
		result = xrEnumerateInstanceExtensionProperties(nullptr, count, &count,
														properties.data());
	}
	if (outResult != nullptr) {
		*outResult = result;
	}
	if (result != XR_SUCCESS) {
		return {};
	}

	std::vector<XrExtensionInfo> extensions;
	extensions.reserve(properties.size());
	for (const auto &property: properties) {
		extensions.push_back(
			{ property.extensionName, property.extensionVersion });
	}
	return extensions;
}

bool evan::XrExtensions::isExtensionSupported(
	const std::vector<XrExtensionInfo> &available,
	std::string_view name) noexcept
{
	for (const auto &extension: available) {
		if (extension.name == name) {
			return true;
		}
	}
	return false;
}

std::optional<std::uint32_t> evan::XrExtensions::extensionVersion(
	const std::vector<XrExtensionInfo> &available,
	std::string_view name) noexcept
{
	for (const auto &extension: available) {
		if (extension.name == name) {
			return extension.version;
		}
	}
	return std::nullopt;
}

std::span<const std::string_view>
	evan::XrExtensions::arExtensionNames() noexcept
{
	static const std::array<std::string_view, 10> arExtensions = {
		"XR_KHR_composition_layer_passthrough",
		"XR_FB_passthrough",
		"XR_EXT_hand_tracking",
		"XR_FB_hand_tracking_mesh",
		"XR_FB_scene",
		"XR_FB_spatial_entity",
		"XR_FB_spatial_entity_query",
		"XR_FB_spatial_entity_storage",
		"XR_FB_composition_layer_depth",
		"XR_KHR_composition_layer_depth",
	};
	return arExtensions;
}

std::vector<std::string_view>
	evan::XrExtensions::extensionsForFeatures(ArFeature features)
{
	std::vector<std::string_view> extensions;

	auto add = [&](std::string_view name) {
		if (std::find(extensions.begin(), extensions.end(), name)
			== extensions.end()) {
			extensions.push_back(name);
		}
	};

	if (hasFeature(features, ArFeature::Passthrough)) {
		add("XR_KHR_composition_layer_passthrough");
		add("XR_FB_passthrough");
	}
	if (hasFeature(features, ArFeature::HandTracking)
		|| hasFeature(features, ArFeature::HandMesh)) {
		add("XR_EXT_hand_tracking");
	}
	if (hasFeature(features, ArFeature::HandMesh)) {
		add("XR_FB_hand_tracking_mesh");
	}
	if (hasFeature(features, ArFeature::Scene)) {
		add("XR_FB_scene");
	}
	if (hasFeature(features, ArFeature::Anchors)) {
		add("XR_FB_spatial_entity");
		add("XR_FB_spatial_entity_query");
		add("XR_FB_spatial_entity_storage");
	}
	if (hasFeature(features, ArFeature::Depth)) {
		add("XR_FB_composition_layer_depth");
		add("XR_KHR_composition_layer_depth");
	}

	return extensions;
}

bool evan::XrExtensions::isArExtensionName(std::string_view name) noexcept
{
	for (const auto &extension: arExtensionNames()) {
		if (extension == name) {
			return true;
		}
	}
	return false;
}

std::vector<std::string> evan::XrExtensions::composeRequiredInstanceExtensions(
	const std::vector<std::string> &baseline, bool enableArExtensions,
	ArFeature features, const std::vector<XrExtensionInfo> &available)
{
	std::vector<std::string> requiredExtensions = baseline;
	if (!enableArExtensions || features == ArFeature::None) {
		return requiredExtensions;
	}

	auto add = [&](std::string_view name) {
		if (std::find(requiredExtensions.begin(), requiredExtensions.end(),
					  name)
			== requiredExtensions.end()) {
			requiredExtensions.emplace_back(name);
		}
	};

	for (const auto name: extensionsForFeatures(features)) {
		if (isExtensionSupported(available, name)) {
			add(name);
		}
	}
	return requiredExtensions;
}
