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
		/**
		 * @brief Extension name reported by the OpenXR runtime.
		 * @details This is the name of the extension as reported by the OpenXR
		 * runtime.
		 */
		std::string name;
		/**
		 * @brief
		 * @details This is the version of the extension as reported by the
		 * OpenXR runtime.
		 */
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
			/**
			 * @brief Creates a passthrough object.
			 *
			 * @param session The session to create the passthrough object in.
			 * @param info The information for the passthrough object.
			 * @param out The output for the created passthrough object.
			 * @return The result of the operation.
			 */
			PFN_xrCreatePassthroughFB create = nullptr;

			/**
			 * @brief Destroys a passthrough object.
			 *
			 * @param passthrough The passthrough object to destroy.
			 * @return The result of the operation.
			 */
			PFN_xrDestroyPassthroughFB destroy = nullptr;

			/**
			 * @brief Starts passthrough.
			 * @param passthrough The passthrough object to start.
			 * @return The result of the operation.
			 */
			PFN_xrPassthroughStartFB start = nullptr;

			/**
			 * @brief Pauses passthrough.
			 * @param passthrough The passthrough object to pause.
			 * @return The result of the operation.
			 */
			PFN_xrPassthroughPauseFB pause = nullptr;

			/**
			 * @brief Creates a passthrough layer.
			 * @param session The session to create the layer in.
			 * @param info The information for the layer.
			 * @param out The output for the created layer.
			 * @return The result of the operation.
			 */
			PFN_xrCreatePassthroughLayerFB createLayer = nullptr;

			/**
			 * @brief Destroys a passthrough layer.
			 * @param layer The layer to destroy.
			 * @return The result of the operation.
			 */
			PFN_xrDestroyPassthroughLayerFB destroyLayer = nullptr;

			/**
			 * @brief Pauses a passthrough layer.
			 * @param layer The layer to pause.
			 * @return The result of the operation.
			 */
			PFN_xrPassthroughLayerPauseFB pauseLayer = nullptr;

			/**
			 * @brief Resumes a passthrough layer.
			 * @param layer The layer to resume.
			 * @return The result of the operation.
			 */
			PFN_xrPassthroughLayerResumeFB resumeLayer = nullptr;

			/**
			 * @brief Sets passthrough layer style.
			 * @param layer The layer to set the style for.
			 * @param style The style to set.
			 * @return The result of the operation.
			 */
			PFN_xrPassthroughLayerSetStyleFB setStyle = nullptr;

			/**
			 * @brief Whether all XR_FB_passthrough functions are present.
			 */
			[[nodiscard]] bool isComplete() const noexcept;
		} passthrough;

		/**
		 * @brief XR_EXT_hand_tracking functions.
		 */
		struct HandTracking {
			/**
			 * @brief Creates a hand tracker.
			 * @param session The session to create the hand tracker in.
			 * @param info The information for the hand tracker.
			 * @param out The output for the created hand tracker.
			 * @return The result of the operation.
			 */
			PFN_xrCreateHandTrackerEXT createTracker = nullptr;

			/**
			 * @brief Destroys a hand tracker.
			 * @param tracker The hand tracker to destroy.
			 * @return The result of the operation.
			 */
			PFN_xrDestroyHandTrackerEXT destroyTracker = nullptr;

			/**
			 * @brief Locates hand joints.
			 * @param tracker The hand tracker to locate joints for.
			 * @param info The information for the joint locations.
			 * @return The result of the operation.
			 */
			PFN_xrLocateHandJointsEXT locateJoints = nullptr;

			/**
			 * @brief Whether all XR_EXT_hand_tracking functions are present.
			 */
			[[nodiscard]] bool isComplete() const noexcept;
		} handTracking;

		/**
		 * @brief XR_FB_hand_tracking_mesh functions.
		 */
		struct HandMesh {
			/**
			 * @brief Retrieves the hand mesh.
			 * @param tracker The hand tracker to retrieve the mesh for.
			 * @return The result of the operation.
			 */
			PFN_xrGetHandMeshFB getMesh = nullptr;

			/**
			 * @brief Whether the hand-mesh function is present.
			 * @return True if the function is present, false otherwise.
			 */
			[[nodiscard]] bool isComplete() const noexcept;
		} handMesh;

		/**
		 * @brief XR_FB_scene functions.
		 */
		struct Scene {
			/**
			 * @brief Retrieves a space's 2D bounding box.
			 * @param space The space to retrieve the bounding box for.
			 * @param out The output for the retrieved bounding box.
			 * @return The result of the operation.
			 */
			PFN_xrGetSpaceBoundingBox2DFB getBoundingBox2D = nullptr;

			/**
			 * @brief Retrieves a space's 3D bounding box.
			 * @param space The space to retrieve the bounding box for.
			 * @param out The output for the retrieved bounding box.
			 * @return The result of the operation.
			 */
			PFN_xrGetSpaceBoundingBox3DFB getBoundingBox3D = nullptr;

			/**
			 * @brief Retrieves semantic labels for a space.
			 * @param space The space to retrieve the semantic labels for.
			 * @param out The output for the retrieved semantic labels.
			 * @return The result of the operation.
			 */
			PFN_xrGetSpaceSemanticLabelsFB getSemanticLabels = nullptr;

			/**
			 * @brief Retrieves a space's 2D boundary.
			 * @param space The space to retrieve the boundary for.
			 * @param out The output for the retrieved boundary.
			 * @return The result of the operation.
			 */
			PFN_xrGetSpaceBoundary2DFB getBoundary2D = nullptr;

			/**
			 * @brief Retrieves a space's room layout.
			 * @param space The space to retrieve the room layout for.
			 * @param out The output for the retrieved room layout.
			 * @return The result of the operation.
			 */
			PFN_xrGetSpaceRoomLayoutFB getRoomLayout = nullptr;

			/**
			 * @brief Whether all XR_FB_scene functions are present.
			 */
			[[nodiscard]] bool isComplete() const noexcept;
		} scene;

		/**
		 * @brief XR_FB_spatial_entity functions.
		 */
		struct SpatialEntity {
			/**
			 * @brief Creates a spatial anchor.
			 *
			 * @param session The session to create the anchor in.
			 * @param info The information for the anchor.
			 * @param out The output for the created anchor.
			 * @return The result of the operation.
			 */
			PFN_xrCreateSpatialAnchorFB createAnchor = nullptr;

			/**
			 * @brief Retrieves a space's UUID.
			 * @param space The space to retrieve the UUID for.
			 * @param out The output for the retrieved UUID.
			 * @return The result of the operation.
			 */
			PFN_xrGetSpaceUuidFB getUuid = nullptr;

			/**
			 * @brief Enumerates a space's supported components.
			 * @param space The space to enumerate the components for.
			 * @param out The output for the enumerated components.
			 * @return The result of the operation.
			 */
			PFN_xrEnumerateSpaceSupportedComponentsFB enumerateComponents =
				nullptr;

			/**
			 * @brief Sets a space component's status.
			 * @param space The space to set the component status for.
			 * @param component The component to set the status for.
			 * @param status The status to set for the component.
			 * @return The result of the operation.
			 */
			PFN_xrSetSpaceComponentStatusFB setComponentStatus = nullptr;

			/**
			 * @brief Retrieves a space component's status.
			 * @param space The space to retrieve the component status for.
			 * @param component The component to retrieve the status for.
			 * @param out The output for the retrieved status.
			 * @return The result of the operation.
			 */
			PFN_xrGetSpaceComponentStatusFB getComponentStatus = nullptr;

			/**
			 * @brief Whether all XR_FB_spatial_entity functions are present.
			 * @return True when every spatial-entity function was resolved.
			 */
			[[nodiscard]] bool isComplete() const noexcept;
		} spatialEntity;

		/**
		 * @brief XR_FB_spatial_entity_query functions.
		 */
		struct SpatialQuery {
			/**
			 * @brief Queries spaces.
			 * @param session The session to query spaces in.
			 * @param info The information for the query.
			 * @return The result of the operation.
			 */
			PFN_xrQuerySpacesFB query = nullptr;

			/**
			 * @brief Retrieves space query results.
			 * @param session The session to retrieve query results in.
			 * @param info The information for the query.
			 * @return The result of the operation.
			 */
			PFN_xrRetrieveSpaceQueryResultsFB retrieveResults = nullptr;

			/**
			 * @brief Whether all XR_FB_spatial_entity_query functions are
			 * present.
			 */
			[[nodiscard]] bool isComplete() const noexcept;
		} spatialQuery;

		/**
		 * @brief XR_FB_spatial_entity_storage functions.
		 */
		struct SpatialStorage {
			/**
			 * @brief Saves a spatial entity.
			 * @param space The space to save.
			 * @return The result of the operation.
			 */
			PFN_xrSaveSpaceFB save = nullptr;

			/**
			 * @brief Erases a saved spatial entity.
			 * @param space The space to erase.
			 * @return The result of the operation.
			 */
			PFN_xrEraseSpaceFB erase = nullptr;

			/**
			 * @brief Whether all XR_FB_spatial_entity_storage functions are
			 * present.
			 */
			[[nodiscard]] bool isComplete() const noexcept;
		} spatialStorage;

		/**
		 * @brief Whether all XR_FB_passthrough functions are present.
		 * @return True when every passthrough function was resolved.
		 */
		[[nodiscard]] bool hasPassthrough() const noexcept
		{
			return passthrough.isComplete();
		}

		/**
		 * @brief Whether all XR_EXT_hand_tracking functions are present.
		 * @return True when every hand-tracking function was resolved.
		 */
		[[nodiscard]] bool hasHandTracking() const noexcept
		{
			return handTracking.isComplete();
		}

		/**
		 * @brief Whether all XR_FB_hand_tracking_mesh functions are present.
		 * @return True when the hand-mesh function was resolved.
		 */
		[[nodiscard]] bool hasHandMesh() const noexcept
		{
			return handMesh.isComplete();
		}

		/**
		 * @brief Whether all XR_FB_scene functions are present.
		 * @return True when every scene function was resolved.
		 */
		[[nodiscard]] bool hasScene() const noexcept
		{
			return scene.isComplete();
		}

		/**
		 * @brief Whether all XR_FB_spatial_entity functions are present.
		 * @return True when every spatial-entity function was resolved.
		 */
		[[nodiscard]] bool hasSpatialEntity() const noexcept
		{
			return spatialEntity.isComplete();
		}

		/**
		 * @brief Whether all XR_FB_spatial_entity_query functions are present.
		 * @return True when every spatial-query function was resolved.
		 */
		[[nodiscard]] bool hasSpatialQuery() const noexcept
		{
			return spatialQuery.isComplete();
		}

		/**
		 * @brief Whether all XR_FB_spatial_entity_storage functions are
		 * present.
		 * @return True when every spatial-storage function was resolved.
		 */
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
