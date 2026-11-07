/*
** ETIB PROJECT, 2026
** evan
** File description:
** GeometryCache
*/

#include "evan/GeometryCache.hpp"

#include "evan/DeviceContext.hpp"
#include "evan/GPUMesh.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <utility>

namespace
{
	/**
	 * @brief Basis of the 64-bit FNV-1a hash used for cache keys.
	 */
	constexpr uint64_t kFnvOffsetBasis = 14695981039346656037ULL;

	/**
	 * @brief Prime of the 64-bit FNV-1a hash used for cache keys.
	 */
	constexpr uint64_t kFnvPrime = 1099511628211ULL;

	/**
	 * @brief Multiplier of the second hash used as collision guard.
	 */
	constexpr uint64_t kSecondHashSeed = 5381ULL;

	/**
	 * @brief Folds @p size bytes of @p data into the FNV-1a hash @p hash.
	 */
	uint64_t fnv1a(uint64_t hash, const void *data, std::size_t size)
	{
		const auto *bytes = static_cast<const unsigned char *>(data);

		for (std::size_t i = 0; i < size; ++i) {
			hash ^= bytes[i];
			hash *= kFnvPrime;
		}
		return hash;
	}

	/**
	 * @brief Folds @p size bytes of @p data into the second hash @p hash.
	 *
	 * A different algorithm from fnv1a() on purpose: the two hashes over the
	 * same content are stored together, so reusing the same mesh requires
	 * hitting both of them.
	 */
	uint64_t secondHash(uint64_t hash, const void *data, std::size_t size)
	{
		const auto *bytes = static_cast<const unsigned char *>(data);

		for (std::size_t i = 0; i < size; ++i) {
			hash = hash * kSecondHashSeed + bytes[i];
		}
		return hash;
	}

	/**
	 * @brief Hashes the attributes of every vertex through @p fold.
	 *
	 * The attributes are hashed one scalar array at a time instead of hashing
	 * the whole `GPUVertex` as bytes: the structure has no padding today, and
	 * this keeps the hash exact whatever its layout becomes.
	 */
	template<typename Fold>
	uint64_t foldVertices(uint64_t hash,
						  const std::vector<evan::GPUVertex> &vertices,
						  Fold fold)
	{
		for (const auto &vertex: vertices) {
			hash = fold(hash, glm::value_ptr(vertex.pos), 3 * sizeof(float));
			hash = fold(hash, glm::value_ptr(vertex.color), 3 * sizeof(float));
			hash =
				fold(hash, glm::value_ptr(vertex.texCoord), 2 * sizeof(float));
		}
		return hash;
	}

	/**
	 * @brief Folds the counts and the material of a content into @p hash.
	 */
	template<typename Fold>
	uint64_t foldCounts(uint64_t hash, std::size_t vertexCount,
						std::size_t indexCount, uint32_t materialID, Fold fold)
	{
		hash = fold(hash, &vertexCount, sizeof(vertexCount));
		hash = fold(hash, &indexCount, sizeof(indexCount));
		return fold(hash, &materialID, sizeof(materialID));
	}
}	 // namespace

////////////////////
// Public Methods //
////////////////////

std::shared_ptr<evan::GPUMesh> evan::GeometryCache::acquire(
	const std::shared_ptr<DeviceContext> &deviceContext,
	std::vector<GPUVertex> vertices, std::vector<uint32_t> indices,
	uint32_t materialID)
{
	const uint64_t key = computeKey(vertices, indices, materialID);
	const uint64_t fingerprint =
		computeFingerprint(vertices, indices, materialID);
	const std::size_t vertexCount = vertices.size();
	const std::size_t indexCount  = indices.size();

	auto entry = _entries.find(key);
	if (entry != _entries.end()) {
		const std::shared_ptr<GPUMesh> cached = entry->second.mesh.lock();

		// A stored mesh is only reused when it still is the mesh this key was
		// built for: the counts, the material and the second hash all have to
		// match, so a key collision falls through to a rebuild instead of
		// handing back the wrong geometry.
		if (cached && entry->second.fingerprint == fingerprint
			&& entry->second.vertexCount == vertexCount
			&& entry->second.indexCount == indexCount
			&& entry->second.materialID == materialID
			&& cached->getVertexCount() == vertexCount
			&& cached->getIndexCount() == indexCount
			&& cached->getMaterialID() == materialID) {
			return cached;
		}
		eraseReverse(entry->second.owner, key);
		_entries.erase(entry);
	}

	std::shared_ptr<GPUMesh> mesh = std::make_shared<GPUMesh>(
		deviceContext, std::move(vertices), std::move(indices), materialID);

	// Pruning on insertion keeps the map from accumulating the entries of
	// every mesh that has already been freed.
	prune();

	Entry stored;
	stored.mesh				  = mesh;
	stored.owner			  = mesh.get();
	stored.fingerprint		  = fingerprint;
	stored.vertexCount		  = vertexCount;
	stored.indexCount		  = indexCount;
	stored.materialID		  = materialID;
	_entries[key]			  = stored;
	_keysByMesh[stored.owner] = key;
	return mesh;
}

uint64_t evan::GeometryCache::computeKey(const std::vector<GPUVertex> &vertices,
										 const std::vector<uint32_t> &indices,
										 uint32_t materialID)
{
	uint64_t hash = foldVertices(kFnvOffsetBasis, vertices, fnv1a);

	hash = fnv1a(hash, indices.data(), indices.size() * sizeof(uint32_t));
	return foldCounts(hash, vertices.size(), indices.size(), materialID, fnv1a);
}

void evan::GeometryCache::forget(const GPUMesh &mesh)
{
	const auto row = _keysByMesh.find(&mesh);

	if (row == _keysByMesh.end()) {
		return;
	}

	const uint64_t key = row->second;
	_keysByMesh.erase(row);

	const auto entry = _entries.find(key);
	if (entry != _entries.end() && entry->second.owner == &mesh) {
		_entries.erase(entry);
	}
}

void evan::GeometryCache::clear()
{
	_entries.clear();
	_keysByMesh.clear();
}

std::size_t evan::GeometryCache::size() const
{
	return _entries.size();
}

///////////////////////
// Protected Methods //
///////////////////////

uint64_t evan::GeometryCache::computeFingerprint(
	const std::vector<GPUVertex> &vertices,
	const std::vector<uint32_t> &indices, uint32_t materialID)
{
	uint64_t hash = foldVertices(0, vertices, secondHash);

	hash = secondHash(hash, indices.data(), indices.size() * sizeof(uint32_t));
	return foldCounts(hash, vertices.size(), indices.size(), materialID,
					  secondHash);
}

void evan::GeometryCache::prune()
{
	for (auto entry = _entries.begin(); entry != _entries.end();) {
		if (entry->second.mesh.expired()) {
			const uint64_t key = entry->first;

			eraseReverse(entry->second.owner, key);
			entry = _entries.erase(entry);
			continue;
		}
		++entry;
	}
}

void evan::GeometryCache::eraseReverse(const GPUMesh *mesh, uint64_t key)
{
	const auto row = _keysByMesh.find(mesh);

	if (row != _keysByMesh.end() && row->second == key) {
		_keysByMesh.erase(row);
	}
}
