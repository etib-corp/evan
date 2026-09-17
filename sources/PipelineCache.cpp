/*
** ETIB PROJECT, 2026
** evan
** File description:
** PipelineCache
*/

#include "evan/PipelineCache.hpp"

#include <cstring>
#include <fstream>
#include <system_error>

namespace
{
	/**
	 * @brief Size of the header carried by every pipeline cache blob.
	 */
	constexpr std::size_t kHeaderSize =
		sizeof(VkPipelineCacheHeaderVersionOne);

	/**
	 * @brief Suffix of the temporary file used to write the blob atomically.
	 */
	constexpr const char *kTemporarySuffix = ".tmp";
}	 // namespace

VkPipelineCacheHeaderVersionOne evan::PipelineCache::buildHeader(
	const VkPhysicalDeviceProperties &properties)
{
	VkPipelineCacheHeaderVersionOne header {};

	header.headerSize	 = static_cast<uint32_t>(kHeaderSize);
	header.headerVersion = VK_PIPELINE_CACHE_HEADER_VERSION_ONE;
	header.vendorID		 = properties.vendorID;
	header.deviceID		 = properties.deviceID;
	std::memcpy(header.pipelineCacheUUID, properties.pipelineCacheUUID,
				VK_UUID_SIZE);

	return header;
}

bool evan::PipelineCache::isBlobCompatible(
	const std::vector<std::byte> &blob,
	const VkPipelineCacheHeaderVersionOne &expected)
{
	if (blob.size() < kHeaderSize) {
		return false;
	}

	VkPipelineCacheHeaderVersionOne header {};
	std::memcpy(&header, blob.data(), kHeaderSize);

	return header.headerSize >= static_cast<uint32_t>(kHeaderSize)
		&& header.headerVersion == VK_PIPELINE_CACHE_HEADER_VERSION_ONE
		&& header.vendorID == expected.vendorID
		&& header.deviceID == expected.deviceID
		&& std::memcmp(header.pipelineCacheUUID, expected.pipelineCacheUUID,
					   VK_UUID_SIZE)
		== 0;
}

bool evan::PipelineCache::readBlob(const std::filesystem::path &path,
								   std::vector<std::byte> &blob)
{
	blob.clear();
	if (path.empty()) {
		return false;
	}

	std::ifstream stream(path, std::ios::binary | std::ios::ate);
	if (!stream) {
		return false;
	}

	const std::streamoff size = stream.tellg();
	if (size <= 0) {
		return false;
	}

	blob.resize(static_cast<std::size_t>(size));
	stream.seekg(0, std::ios::beg);
	stream.read(reinterpret_cast<char *>(blob.data()),
				static_cast<std::streamsize>(size));

	if (!stream) {
		blob.clear();
		return false;
	}
	return true;
}

bool evan::PipelineCache::writeBlob(const std::filesystem::path &path,
									const std::vector<std::byte> &blob)
{
	if (path.empty() || blob.empty()) {
		return false;
	}

	std::error_code error;
	const std::filesystem::path parent = path.parent_path();
	if (!parent.empty()) {
		std::filesystem::create_directories(parent, error);
	}

	const std::filesystem::path temporary(path.string() + kTemporarySuffix);

	{
		std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
		if (!stream) {
			return false;
		}
		stream.write(reinterpret_cast<const char *>(blob.data()),
					 static_cast<std::streamsize>(blob.size()));
		if (!stream) {
			std::error_code cleanupError;
			std::filesystem::remove(temporary, cleanupError);
			return false;
		}
	}

	std::filesystem::rename(temporary, path, error);
	if (error) {
		// Some platforms refuse to replace an existing file, so drop the
		// stale blob before renaming the new one into place.
		std::error_code removeError;
		std::filesystem::remove(path, removeError);
		error.clear();
		std::filesystem::rename(temporary, path, error);
	}
	if (error) {
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		return false;
	}
	return true;
}

void evan::PipelineCache::init(VkDevice device,
							   VkPhysicalDevice physicalDevice,
							   const std::filesystem::path &path)
{
	_device = device;
	_path	= path;

	if (_device == VK_NULL_HANDLE) {
		this->getLogger().warning()
			<< "No logical device given to the pipeline cache; pipelines "
			   "will be compiled uncached.";
		return;
	}

	VkPhysicalDeviceProperties properties {};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	_expectedHeader = buildHeader(properties);

	VkPipelineCacheCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	std::vector<std::byte> blob;
	const bool reusable = this->isPersistent() && readBlob(_path, blob)
		&& isBlobCompatible(blob, _expectedHeader);

	if (reusable) {
		this->getLogger().info()
			<< "Reusing persisted pipeline cache: " << _path.string() << " ("
			<< blob.size() << " bytes).";
		createInfo.initialDataSize = blob.size();
		createInfo.pInitialData	   = blob.data();
	} else if (_path.empty()) {
		this->getLogger().info()
			<< "Starting with an empty pipeline cache. Persistence is "
			   "disabled.";
	} else {
		this->getLogger().info()
			<< "Starting with an empty pipeline cache. No reusable blob at: "
			<< _path.string();
	}

	if (vkCreatePipelineCache(_device, &createInfo, nullptr, &_handle)
		== VK_SUCCESS) {
		return;
	}

	if (!reusable) {
		this->getLogger().error()
			<< "Failed to create the pipeline cache; pipelines will be "
			   "compiled uncached.";
		_handle = VK_NULL_HANDLE;
		return;
	}

	// A blob that passed our own validation can still be rejected by the
	// driver. Retry without it so pipeline creation stays possible.
	createInfo.initialDataSize = 0;
	createInfo.pInitialData	   = nullptr;
	if (vkCreatePipelineCache(_device, &createInfo, nullptr, &_handle)
		!= VK_SUCCESS) {
		this->getLogger().error()
			<< "Failed to create the pipeline cache; pipelines will be "
			   "compiled uncached.";
		_handle = VK_NULL_HANDLE;
		return;
	}
	this->getLogger().warning()
		<< "The driver rejected the persisted pipeline cache. It was "
		   "discarded and an empty cache was created instead.";
}

void evan::PipelineCache::persist()
{
	if (_handle == VK_NULL_HANDLE || !this->isPersistent()) {
		return;
	}

	std::size_t size = 0;
	if (vkGetPipelineCacheData(_device, _handle, &size, nullptr) != VK_SUCCESS
		|| size == 0) {
		this->getLogger().info()
			<< "Pipeline cache is empty; nothing to persist.";
		return;
	}

	std::vector<std::byte> blob(size);
	if (vkGetPipelineCacheData(_device, _handle, &size, blob.data())
		!= VK_SUCCESS) {
		this->getLogger().warning()
			<< "Failed to read the pipeline cache data.";
		return;
	}
	blob.resize(size);

	if (writeBlob(_path, blob)) {
		this->getLogger().info()
			<< "Persisted pipeline cache: " << _path.string() << " ("
			<< blob.size() << " bytes).";
		return;
	}
	this->getLogger().warning()
		<< "Failed to persist the pipeline cache to: " << _path.string();
}

void evan::PipelineCache::destroy(VkDevice device)
{
	if (_handle == VK_NULL_HANDLE) {
		return;
	}

	vkDestroyPipelineCache(device, _handle, nullptr);
	_handle = VK_NULL_HANDLE;
	_device = VK_NULL_HANDLE;
}
