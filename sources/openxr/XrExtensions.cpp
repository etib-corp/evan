/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrExtensions
*/

#include "evan/openxr/XrExtensions.hpp"

#include <array>

namespace
{
	/**
	 * @brief Resolves one function and records the result.
	 *
	 * A resolver may return XR_SUCCESS with a null pointer when the runtime
	 * does not provide the function; both cases are treated as missing.
	 */
	template<typename Function>
	bool loadFunction(const evan::XrExtensions::ProcAddrResolver &resolver,
					  const char *name, Function &target,
					  evan::XrExtensionLoadReport *report)
	{
		target = nullptr;
		if (!resolver) {
			if (report != nullptr) {
				report->missing.emplace_back(name);
			}
			return false;
		}

		PFN_xrVoidFunction function = nullptr;
		const XrResult result		= resolver(name, &function);
		if (result != XR_SUCCESS || function == nullptr) {
			if (report != nullptr) {
				report->missing.emplace_back(name);
			}
			return false;
		}

		target = reinterpret_cast<Function>(function);
		if (report != nullptr) {
			report->loaded.emplace_back(name);
		}
		return true;
	}
}	 // namespace

bool evan::XrInstanceFunctions::Passthrough::isComplete() const noexcept
{
	return create != nullptr && destroy != nullptr && start != nullptr
		&& pause != nullptr && createLayer != nullptr && destroyLayer != nullptr
		&& pauseLayer != nullptr && resumeLayer != nullptr;
}

bool evan::XrInstanceFunctions::HandTracking::isComplete() const noexcept
{
	return createTracker != nullptr && destroyTracker != nullptr
		&& locateJoints != nullptr;
}

bool evan::XrInstanceFunctions::HandMesh::isComplete() const noexcept
{
	return getMesh != nullptr;
}

bool evan::XrInstanceFunctions::Scene::isComplete() const noexcept
{
	return getBoundingBox2D != nullptr && getBoundingBox3D != nullptr
		&& getSemanticLabels != nullptr && getBoundary2D != nullptr
		&& getRoomLayout != nullptr;
}

bool evan::XrInstanceFunctions::SpatialEntity::isComplete() const noexcept
{
	return createAnchor != nullptr && getUuid != nullptr
		&& enumerateComponents != nullptr && setComponentStatus != nullptr
		&& getComponentStatus != nullptr;
}

bool evan::XrInstanceFunctions::SpatialQuery::isComplete() const noexcept
{
	return query != nullptr && retrieveResults != nullptr;
}

bool evan::XrInstanceFunctions::SpatialStorage::isComplete() const noexcept
{
	return save != nullptr && erase != nullptr;
}

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

evan::XrInstanceFunctions
	evan::XrExtensions::loadInstanceFunctions(XrInstance instance,
											  XrExtensionLoadReport *report)
{
	if (instance == XR_NULL_HANDLE) {
		return loadInstanceFunctions(
			[](const char *, PFN_xrVoidFunction *) {
				return XR_ERROR_HANDLE_INVALID;
			},
			report);
	}

	return loadInstanceFunctions(
		[instance](const char *name, PFN_xrVoidFunction *function) {
			return xrGetInstanceProcAddr(instance, name, function);
		},
		report);
}

evan::XrInstanceFunctions
	evan::XrExtensions::loadInstanceFunctions(const ProcAddrResolver &resolver,
											  XrExtensionLoadReport *report)
{
	XrInstanceFunctions functions;

	auto load = [&](const char *name, auto &target) {
		loadFunction(resolver, name, target, report);
	};

	// XR_FB_passthrough
	load("xrCreatePassthroughFB", functions.passthrough.create);
	load("xrDestroyPassthroughFB", functions.passthrough.destroy);
	load("xrPassthroughStartFB", functions.passthrough.start);
	load("xrPassthroughPauseFB", functions.passthrough.pause);
	load("xrCreatePassthroughLayerFB", functions.passthrough.createLayer);
	load("xrDestroyPassthroughLayerFB", functions.passthrough.destroyLayer);
	load("xrPassthroughLayerPauseFB", functions.passthrough.pauseLayer);
	load("xrPassthroughLayerResumeFB", functions.passthrough.resumeLayer);
	load("xrPassthroughLayerSetStyleFB", functions.passthrough.setStyle);

	// XR_EXT_hand_tracking
	load("xrCreateHandTrackerEXT", functions.handTracking.createTracker);
	load("xrDestroyHandTrackerEXT", functions.handTracking.destroyTracker);
	load("xrLocateHandJointsEXT", functions.handTracking.locateJoints);

	// XR_FB_hand_tracking_mesh
	load("xrGetHandMeshFB", functions.handMesh.getMesh);

	// XR_FB_scene
	load("xrGetSpaceBoundingBox2DFB", functions.scene.getBoundingBox2D);
	load("xrGetSpaceBoundingBox3DFB", functions.scene.getBoundingBox3D);
	load("xrGetSpaceSemanticLabelsFB", functions.scene.getSemanticLabels);
	load("xrGetSpaceBoundary2DFB", functions.scene.getBoundary2D);
	load("xrGetSpaceRoomLayoutFB", functions.scene.getRoomLayout);

	// XR_FB_spatial_entity
	load("xrCreateSpatialAnchorFB", functions.spatialEntity.createAnchor);
	load("xrGetSpaceUuidFB", functions.spatialEntity.getUuid);
	load("xrEnumerateSpaceSupportedComponentsFB",
		 functions.spatialEntity.enumerateComponents);
	load("xrSetSpaceComponentStatusFB",
		 functions.spatialEntity.setComponentStatus);
	load("xrGetSpaceComponentStatusFB",
		 functions.spatialEntity.getComponentStatus);

	// XR_FB_spatial_entity_query
	load("xrQuerySpacesFB", functions.spatialQuery.query);
	load("xrRetrieveSpaceQueryResultsFB",
		 functions.spatialQuery.retrieveResults);

	// XR_FB_spatial_entity_storage
	load("xrSaveSpaceFB", functions.spatialStorage.save);
	load("xrEraseSpaceFB", functions.spatialStorage.erase);

	return functions;
}
