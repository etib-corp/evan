/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrExtensions
*/

#pragma once

#include <openxr/openxr.h>

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace evan
{
	/**
	 * @brief One OpenXR instance extension reported by the loader/runtime.
	 */
	struct XrExtensionInfo {
		std::string name;
		std::uint32_t version = 0;
	};

	/**
	 * @brief Result of loading the AR extension functions.
	 */
	struct XrExtensionLoadReport {
		std::vector<std::string> loaded;	 ///< Functions that resolved.
		std::vector<std::string> missing;	 ///< Functions that did not.
	};

	/**
	 * @brief Typed extension functions resolved for an OpenXR instance.
	 *
	 * A missing extension leaves its pointers null. Consumers must check the
	 * matching has*() helper, or the individual pointer, before calling.
	 */
	struct XrInstanceFunctions {
		/**
		 * @brief XR_FB_passthrough functions.
		 */
		struct Passthrough {
			PFN_xrCreatePassthroughFB create			 = nullptr;
			PFN_xrDestroyPassthroughFB destroy			 = nullptr;
			PFN_xrPassthroughStartFB start				 = nullptr;
			PFN_xrPassthroughPauseFB pause				 = nullptr;
			PFN_xrCreatePassthroughLayerFB createLayer	 = nullptr;
			PFN_xrDestroyPassthroughLayerFB destroyLayer = nullptr;
			PFN_xrPassthroughLayerPauseFB pauseLayer	 = nullptr;
			PFN_xrPassthroughLayerResumeFB resumeLayer	 = nullptr;
			PFN_xrPassthroughLayerSetStyleFB setStyle	 = nullptr;

			[[nodiscard]] bool isComplete() const noexcept;
		} passthrough;

		/**
		 * @brief XR_EXT_hand_tracking functions.
		 */
		struct HandTracking {
			PFN_xrCreateHandTrackerEXT createTracker   = nullptr;
			PFN_xrDestroyHandTrackerEXT destroyTracker = nullptr;
			PFN_xrLocateHandJointsEXT locateJoints	   = nullptr;

			[[nodiscard]] bool isComplete() const noexcept;
		} handTracking;

		/**
		 * @brief XR_FB_hand_tracking_mesh functions.
		 */
		struct HandMesh {
			PFN_xrGetHandMeshFB getMesh = nullptr;

			[[nodiscard]] bool isComplete() const noexcept;
		} handMesh;

		/**
		 * @brief XR_FB_scene functions.
		 */
		struct Scene {
			PFN_xrGetSpaceBoundingBox2DFB getBoundingBox2D	 = nullptr;
			PFN_xrGetSpaceBoundingBox3DFB getBoundingBox3D	 = nullptr;
			PFN_xrGetSpaceSemanticLabelsFB getSemanticLabels = nullptr;
			PFN_xrGetSpaceBoundary2DFB getBoundary2D		 = nullptr;
			PFN_xrGetSpaceRoomLayoutFB getRoomLayout		 = nullptr;

			[[nodiscard]] bool isComplete() const noexcept;
		} scene;

		/**
		 * @brief XR_FB_spatial_entity functions.
		 */
		struct SpatialEntity {
			PFN_xrCreateSpatialAnchorFB createAnchor = nullptr;
			PFN_xrGetSpaceUuidFB getUuid			 = nullptr;
			PFN_xrEnumerateSpaceSupportedComponentsFB enumerateComponents =
				nullptr;
			PFN_xrSetSpaceComponentStatusFB setComponentStatus = nullptr;
			PFN_xrGetSpaceComponentStatusFB getComponentStatus = nullptr;

			[[nodiscard]] bool isComplete() const noexcept;
		} spatialEntity;

		/**
		 * @brief XR_FB_spatial_entity_query functions.
		 */
		struct SpatialQuery {
			PFN_xrQuerySpacesFB query						  = nullptr;
			PFN_xrRetrieveSpaceQueryResultsFB retrieveResults = nullptr;

			[[nodiscard]] bool isComplete() const noexcept;
		} spatialQuery;

		/**
		 * @brief XR_FB_spatial_entity_storage functions.
		 */
		struct SpatialStorage {
			PFN_xrSaveSpaceFB save	 = nullptr;
			PFN_xrEraseSpaceFB erase = nullptr;

			[[nodiscard]] bool isComplete() const noexcept;
		} spatialStorage;

		[[nodiscard]] bool hasPassthrough() const noexcept
		{
			return passthrough.isComplete();
		}

		[[nodiscard]] bool hasHandTracking() const noexcept
		{
			return handTracking.isComplete();
		}

		[[nodiscard]] bool hasHandMesh() const noexcept
		{
			return handMesh.isComplete();
		}

		[[nodiscard]] bool hasScene() const noexcept
		{
			return scene.isComplete();
		}

		[[nodiscard]] bool hasSpatialEntity() const noexcept
		{
			return spatialEntity.isComplete();
		}

		[[nodiscard]] bool hasSpatialQuery() const noexcept
		{
			return spatialQuery.isComplete();
		}

		[[nodiscard]] bool hasSpatialStorage() const noexcept
		{
			return spatialStorage.isComplete();
		}
	};

	/**
	 * @brief OpenXR instance-extension discovery and pure capability queries.
	 */
	class XrExtensions
	{
		public:
		/**
		 * @brief Resolver used to load an instance function by name.
		 */
		using ProcAddrResolver =
			std::function<XrResult(const char *, PFN_xrVoidFunction *)>;

		/**
		 * @brief Enumerates every instance extension available to the loader.
		 *
		 * @param outResult Optional result of the last OpenXR call, for
		 * logging.
		 * @return The extensions, or an empty vector on failure.
		 */
		[[nodiscard]] static std::vector<XrExtensionInfo>
			enumerateInstanceExtensions(XrResult *outResult = nullptr);

		/**
		 * @brief Whether @p name is present in @p available.
		 *
		 * @param available The extension list to search.
		 * @param name The extension name, compared case-sensitively.
		 * @return True when the extension is present.
		 */
		[[nodiscard]] static bool
			isExtensionSupported(const std::vector<XrExtensionInfo> &available,
								 std::string_view name) noexcept;

		/**
		 * @brief Version of @p name when present.
		 */
		[[nodiscard]] static std::optional<std::uint32_t>
			extensionVersion(const std::vector<XrExtensionInfo> &available,
							 std::string_view name) noexcept;

		/**
		 * @brief AR-relevant extension names from the roadmap §13.
		 *
		 * Kept as string literals so a missing vendor header never breaks the
		 * build. AR-008 decides which of them to enable.
		 */
		[[nodiscard]] static std::span<const std::string_view>
			arExtensionNames() noexcept;

		/**
		 * @brief Resolves all AR extension functions for @p instance.
		 *
		 * Missing functions are left null and listed in @p report. The call is
		 * safe with a null instance.
		 *
		 * @param instance The OpenXR instance.
		 * @param report Optional loaded/missing report.
		 * @return The typed function table.
		 */
		[[nodiscard]] static XrInstanceFunctions
			loadInstanceFunctions(XrInstance instance,
								  XrExtensionLoadReport *report = nullptr);

		/**
		 * @brief Resolves all AR extension functions through @p resolver.
		 *
		 * Test seam: lets callers supply the function resolver without a real
		 * OpenXR instance.
		 *
		 * @param resolver The function resolver.
		 * @param report Optional loaded/missing report.
		 * @return The typed function table.
		 */
		[[nodiscard]] static XrInstanceFunctions
			loadInstanceFunctions(const ProcAddrResolver &resolver,
								  XrExtensionLoadReport *report = nullptr);
	};
}	 // namespace evan
