/*
** ETIB PROJECT, 2026
** evan
** File description:
** GeometryCache
*/

#pragma once

#include "evan/GPUVertex.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

namespace evan
{
	class DeviceContext;
	class GPUMesh;

	/**
	 * @brief Shares GPU geometry between objects built from identical meshes.
	 *
	 * Every `GPUMesh` owns one vertex and one index buffer. Objects built
	 * from the same source mesh used to upload the same bytes once each,
	 * which gave every copy a distinct buffer handle: the renderer could not
	 * merge them back into a single draw because merging requires the same
	 * buffers.
	 *
	 * The cache removes that duplication. A mesh is identified by a hash of
	 * its content (vertex bytes, index bytes, both counts) and of the
	 * material ID it was built for. The first request for a content builds
	 * the `GPUMesh` and stores a weak reference to it; every later request
	 * for the same content returns the same object, so the buffers are
	 * allocated once.
	 *
	 * @note The cache never owns geometry: it only keeps `std::weak_ptr`
	 * references, so dropping the last user frees the buffers. Expired
	 * entries are pruned whenever the cache is inserted into.
	 *
	 * @note The material ID is part of the key because a `GPUMesh` records
	 * the material it was built with; two objects with identical vertices
	 * but different materials must not share.
	 *
	 * @note Meshes returned by the cache may be shared. A user that mutates
	 * one in place must first take ownership (see
	 * `RenderObject::updateMeshes`) and drop its cache entry, otherwise the
	 * content no longer matches the key it is stored under.
	 */
	class GeometryCache
	{
		public:
		GeometryCache() = default;

		~GeometryCache() = default;

		/**
		 * @brief Retrieves the mesh for a content, building it once.
		 *
		 * A hit locks the stored weak reference and verifies the counts, the
		 * material ID and a second, independent fingerprint before reusing
		 * it, so a 64-bit hash collision can never hand back the geometry of
		 * another mesh. A miss (or a rejected entry) builds a new `GPUMesh`
		 * and stores it.
		 *
		 * @param deviceContext Device used to allocate the buffers when the
		 * content is not cached yet.
		 * @param vertices Vertex data of the mesh, moved into the new
		 * `GPUMesh` on a miss.
		 * @param indices Index data of the mesh, moved into the new `GPUMesh`
		 * on a miss.
		 * @param materialID Material the mesh is built for.
		 * @return The shared mesh: the cached one when its content matches,
		 * a newly built one otherwise.
		 */
		[[nodiscard]] std::shared_ptr<GPUMesh>
			acquire(const std::shared_ptr<DeviceContext> &deviceContext,
					std::vector<GPUVertex> vertices,
					std::vector<uint32_t> indices, uint32_t materialID);

		/**
		 * @brief Computes the cache key of a mesh content.
		 *
		 * The key covers the vertex attributes, the index data, both element
		 * counts and the material ID, so two meshes share a key only when
		 * they would produce the same buffers.
		 *
		 * @param vertices Vertex data of the mesh.
		 * @param indices Index data of the mesh.
		 * @param materialID Material the mesh is built for.
		 * @return The 64-bit cache key.
		 */
		[[nodiscard]] static uint64_t
			computeKey(const std::vector<GPUVertex> &vertices,
					   const std::vector<uint32_t> &indices,
					   uint32_t materialID);

		/**
		 * @brief Drops every entry pointing at @p mesh.
		 *
		 * Must be called before a cached mesh is modified in place: its
		 * content changes, so the entry it is stored under would otherwise
		 * hand this mesh to a later request for the old content.
		 *
		 * @param mesh The mesh whose entries are dropped.
		 */
		void forget(const GPUMesh &mesh);

		/**
		 * @brief Drops every entry of the cache.
		 *
		 * The meshes themselves are not affected: the cache holds no
		 * ownership.
		 */
		void clear();

		/**
		 * @brief Retrieves the number of entries currently stored.
		 *
		 * Entries whose mesh was released are still counted until the next
		 * insertion or forget() prunes them.
		 *
		 * @return The number of entries in the cache.
		 */
		[[nodiscard]] std::size_t size() const;

		protected:
		/**
		 * @brief Description of a cached mesh, used to validate a hit.
		 */
		struct Entry {
			std::weak_ptr<GPUMesh> mesh;	   ///< The cached mesh, if alive.
			const GPUMesh *owner = nullptr;	   ///< Address of that mesh, used
											   ///< to drop the reverse index
											   ///< entry without locking it.
			uint64_t fingerprint = 0;		///< Second, independent hash of
											///< the content.
			std::size_t vertexCount = 0;	///< Vertex count of the content.
			std::size_t indexCount	= 0;	///< Index count of the content.
			uint32_t materialID		= 0;	///< Material of the content.
		};

		/**
		 * @brief Computes the fingerprint stored next to a cache key.
		 *
		 * A different algorithm over the same bytes as computeKey(), so a
		 * collision needs to hit both hashes at once.
		 *
		 * @param vertices Vertex data of the mesh.
		 * @param indices Index data of the mesh.
		 * @param materialID Material the mesh is built for.
		 * @return The 64-bit fingerprint.
		 */
		[[nodiscard]] static uint64_t
			computeFingerprint(const std::vector<GPUVertex> &vertices,
							   const std::vector<uint32_t> &indices,
							   uint32_t materialID);

		/**
		 * @brief Erases the entries whose mesh has been released.
		 */
		void prune();

		/**
		 * @brief Erases the reverse index entry of a removed mesh.
		 *
		 * The row is only dropped when it still maps @p mesh to @p key, so a
		 * mesh that was allocated at a recycled address keeps its own row.
		 *
		 * @param mesh Address of the removed mesh.
		 * @param key The cache key it was stored under.
		 */
		void eraseReverse(const GPUMesh *mesh, uint64_t key);

		/**
		 * @brief Entries of the cache, indexed by content key.
		 */
		std::map<uint64_t, Entry> _entries;

		/**
		 * @brief Cache key of every mesh, indexed by its address.
		 *
		 * Lets forget() find the entry to drop without scanning the whole
		 * cache, so rewriting a mesh in place stays O(1) whatever the scene
		 * holds.
		 */
		std::unordered_map<const GPUMesh *, uint64_t> _keysByMesh;
	};
}	 // namespace evan
