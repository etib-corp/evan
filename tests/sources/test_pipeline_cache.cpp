/*
 Copyright (c) 2026 ETIB Corporation

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include <evan/IPlatform.hpp>
#include <evan/PipelineCache.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace xider::tests
{
	namespace
	{
		/**
		 * @brief Cache directory override read by the platform.
		 */
		constexpr const char *kCacheDirVariable = "EVAN_PIPELINE_CACHE_DIR";

		/**
		 * @brief Kill switch read by the platform.
		 */
		constexpr const char *kDisableVariable = "EVAN_PIPELINE_CACHE_DISABLE";

		/**
		 * @brief Reads an environment variable.
		 *
		 * @param name Name of the variable to read.
		 * @return The value of the variable, or nullopt when it is not set.
		 */
		std::optional<std::string> getEnvironment(const std::string &name)
		{
			const char *value = std::getenv(name.c_str());
			if (value == nullptr) {
				return std::nullopt;
			}
			return std::string(value);
		}

		/**
		 * @brief Sets an environment variable.
		 *
		 * @param name Name of the variable to set.
		 * @param value Value to assign to the variable.
		 */
		void setEnvironment(const std::string &name, const std::string &value)
		{
#ifdef _WIN32
			_putenv_s(name.c_str(), value.c_str());
#else
			setenv(name.c_str(), value.c_str(), 1);
#endif
		}

		/**
		 * @brief Removes an environment variable.
		 *
		 * @param name Name of the variable to remove.
		 */
		void unsetEnvironment(const std::string &name)
		{
#ifdef _WIN32
			_putenv_s(name.c_str(), "");
#else
			unsetenv(name.c_str());
#endif
		}

		/**
		 * @brief Assigns an environment variable for the duration of a scope.
		 *
		 * The previous value, if any, is restored when the scope exits so the
		 * tests never leak into the environment of the developer running them.
		 */
		class ScopedEnvironmentVariable
		{
			public:
			ScopedEnvironmentVariable(const std::string &name,
									  const std::string &value)
				: _name(name)
				, _previous(getEnvironment(_name))
			{
				setEnvironment(_name, value);
			}

			~ScopedEnvironmentVariable()
			{
				if (_previous.has_value()) {
					setEnvironment(_name, *_previous);
					return;
				}
				unsetEnvironment(_name);
			}

			ScopedEnvironmentVariable(const ScopedEnvironmentVariable &) =
				delete;
			ScopedEnvironmentVariable &
			operator=(const ScopedEnvironmentVariable &) = delete;
			ScopedEnvironmentVariable(ScopedEnvironmentVariable &&) = delete;

			private:
			std::string _name;
			std::optional<std::string> _previous;
		};

		/**
		 * @brief Creates an empty temporary directory removed on scope exit.
		 */
		class ScopedTemporaryDirectory
		{
			public:
			ScopedTemporaryDirectory()
			{
				std::error_code error;
				_path = std::filesystem::temp_directory_path(error)
					/ ("evan-pipeline-cache-"
					   + std::to_string(std::chrono::steady_clock::now()
											.time_since_epoch()
											.count()));
				std::filesystem::remove_all(_path, error);
				std::filesystem::create_directories(_path, error);
			}

			~ScopedTemporaryDirectory()
			{
				std::error_code error;
				std::filesystem::remove_all(_path, error);
			}

			ScopedTemporaryDirectory(const ScopedTemporaryDirectory &) =
				delete;
			ScopedTemporaryDirectory &
			operator=(const ScopedTemporaryDirectory &) = delete;
			ScopedTemporaryDirectory(ScopedTemporaryDirectory &&) = delete;

			[[nodiscard]] const std::filesystem::path &path() const
			{
				return _path;
			}

			private:
			std::filesystem::path _path;
		};

		/**
		 * @brief Minimal platform exposing the protected cache root.
		 *
		 * The pipeline cache path only depends on the cache root and on the
		 * environment, so the device related entry points are never called and
		 * can stay empty.
		 */
		class TestablePlatform: public evan::IPlatform
		{
			public:
			explicit TestablePlatform(std::filesystem::path cacheRoot)
				: _cacheRoot(std::move(cacheRoot))
			{
			}

			std::vector<std::string>
				getRequiredInstanceExtensions() const override
			{
				return {};
			}

			bool shouldClose() const override { return false; }

			std::vector<std::shared_ptr<utility::event::Event>>
				pollEvents(evan::ADeviceBackend &) override
			{
				return {};
			}

			std::shared_ptr<evan::ADeviceBackend>
				createDeviceBackend() const override
			{
				return nullptr;
			}

			std::shared_ptr<evan::ASwapchainContext> createSwapchainContext(
				const evan::DeviceContext &) const override
			{
				return nullptr;
			}

			protected:
			std::filesystem::path getDefaultCacheRoot() const override
			{
				return _cacheRoot;
			}

			private:
			std::filesystem::path _cacheRoot;
		};

		/**
		 * @brief Builds a physical device property set with a given identity.
		 *
		 * @param vendorID Vendor identifier to report.
		 * @param deviceID Device identifier to report.
		 * @param uuidSeed Value the pipeline cache UUID is filled with.
		 * @return The properties of the fake device.
		 */
		VkPhysicalDeviceProperties
			makeProperties(uint32_t vendorID, uint32_t deviceID, uint8_t uuidSeed)
		{
			VkPhysicalDeviceProperties properties {};
			properties.vendorID = vendorID;
			properties.deviceID = deviceID;
			for (std::size_t index = 0; index < VK_UUID_SIZE; ++index) {
				properties.pipelineCacheUUID[index] =
					static_cast<uint8_t>(uuidSeed + index);
			}
			return properties;
		}

		/**
		 * @brief Builds a blob whose header matches a device.
		 *
		 * @param properties Properties of the device that produced the blob.
		 * @param trailingBytes Number of opaque bytes appended after the
		 * header, mimicking the payload a driver writes.
		 * @return The raw bytes of a persisted cache.
		 */
		std::vector<std::byte> makeBlob(
			const VkPhysicalDeviceProperties &properties,
			std::size_t trailingBytes = 32)
		{
			const auto header = evan::PipelineCache::buildHeader(properties);
			std::vector<std::byte> blob(sizeof(VkPipelineCacheHeaderVersionOne)
										+ trailingBytes);
			std::memcpy(blob.data(), &header,
						sizeof(VkPipelineCacheHeaderVersionOne));
			return blob;
		}
	}	 // namespace

	TEST(PipelineCacheTest, BuildHeaderUsesDeviceProperties)
	{
		const auto properties = makeProperties(0x10DE, 0x2484, 7);

		const auto header = evan::PipelineCache::buildHeader(properties);

		EXPECT_EQ(header.headerSize,
				  static_cast<uint32_t>(sizeof(VkPipelineCacheHeaderVersionOne)));
		EXPECT_EQ(header.headerVersion, VK_PIPELINE_CACHE_HEADER_VERSION_ONE);
		EXPECT_EQ(header.vendorID, properties.vendorID);
		EXPECT_EQ(header.deviceID, properties.deviceID);
		EXPECT_EQ(std::memcmp(header.pipelineCacheUUID,
							  properties.pipelineCacheUUID, VK_UUID_SIZE),
				  0);
	}

	TEST(PipelineCacheTest, AcceptsBlobFromTheSameDevice)
	{
		const auto properties = makeProperties(0x10DE, 0x2484, 7);

		EXPECT_TRUE(evan::PipelineCache::isBlobCompatible(
			makeBlob(properties), evan::PipelineCache::buildHeader(properties)));
	}

	TEST(PipelineCacheTest, RejectsEmptyBlob)
	{
		const auto properties	 = makeProperties(0x10DE, 0x2484, 7);
		const std::vector<std::byte> blob;

		EXPECT_FALSE(evan::PipelineCache::isBlobCompatible(
			blob, evan::PipelineCache::buildHeader(properties)));
	}

	TEST(PipelineCacheTest, RejectsTruncatedBlob)
	{
		const auto properties = makeProperties(0x10DE, 0x2484, 7);
		std::vector<std::byte> blob = makeBlob(properties);
		blob.resize(sizeof(VkPipelineCacheHeaderVersionOne) - 1);

		EXPECT_FALSE(evan::PipelineCache::isBlobCompatible(
			blob, evan::PipelineCache::buildHeader(properties)));
	}

	TEST(PipelineCacheTest, RejectsBlobWithUnknownHeaderVersion)
	{
		const auto properties = makeProperties(0x10DE, 0x2484, 7);
		VkPipelineCacheHeaderVersionOne header =
			evan::PipelineCache::buildHeader(properties);
		header.headerVersion = VK_PIPELINE_CACHE_HEADER_VERSION_MAX_ENUM;
		std::vector<std::byte> blob(sizeof(header) + 8);
		std::memcpy(blob.data(), &header, sizeof(header));

		EXPECT_FALSE(evan::PipelineCache::isBlobCompatible(
			blob, evan::PipelineCache::buildHeader(properties)));
	}

	TEST(PipelineCacheTest, RejectsBlobFromAnotherVendor)
	{
		const auto properties = makeProperties(0x10DE, 0x2484, 7);

		EXPECT_FALSE(evan::PipelineCache::isBlobCompatible(
			makeBlob(makeProperties(0x1002, 0x2484, 7)),
			evan::PipelineCache::buildHeader(properties)));
	}

	TEST(PipelineCacheTest, RejectsBlobFromAnotherDevice)
	{
		const auto properties = makeProperties(0x10DE, 0x2484, 7);

		EXPECT_FALSE(evan::PipelineCache::isBlobCompatible(
			makeBlob(makeProperties(0x10DE, 0x1F91, 7)),
			evan::PipelineCache::buildHeader(properties)));
	}

	TEST(PipelineCacheTest, RejectsBlobFromAnotherDriverVersion)
	{
		const auto properties = makeProperties(0x10DE, 0x2484, 7);
		std::vector<std::byte> blob = makeBlob(properties);
		const std::size_t lastUuidByte =
			sizeof(VkPipelineCacheHeaderVersionOne) - 1;
		blob[lastUuidByte] =
			static_cast<std::byte>(std::to_integer<uint8_t>(blob[lastUuidByte])
								   ^ 0x01);

		EXPECT_FALSE(evan::PipelineCache::isBlobCompatible(
			blob, evan::PipelineCache::buildHeader(properties)));
	}

	TEST(PipelineCacheTest, RoundTripsBlobOnDisk)
	{
		ScopedTemporaryDirectory directory;
		const auto path = directory.path() / "nested" / "pipeline.bin";
		const auto properties = makeProperties(0x10DE, 0x2484, 7);
		const std::vector<std::byte> written = makeBlob(properties);

		ASSERT_TRUE(evan::PipelineCache::writeBlob(path, written));

		std::vector<std::byte> read;
		ASSERT_TRUE(evan::PipelineCache::readBlob(path, read));
		EXPECT_EQ(read, written);
		EXPECT_TRUE(evan::PipelineCache::isBlobCompatible(
			read, evan::PipelineCache::buildHeader(properties)));
	}

	TEST(PipelineCacheTest, WriteBlobReplacesAnExistingBlob)
	{
		ScopedTemporaryDirectory directory;
		const auto path = directory.path() / "pipeline.bin";
		const auto properties = makeProperties(0x10DE, 0x2484, 7);
		const auto firstBlob = makeBlob(properties);
		const auto secondBlob = makeBlob(properties, 64);

		ASSERT_TRUE(evan::PipelineCache::writeBlob(path, firstBlob));
		ASSERT_TRUE(evan::PipelineCache::writeBlob(path, secondBlob));

		std::vector<std::byte> read;
		ASSERT_TRUE(evan::PipelineCache::readBlob(path, read));
		EXPECT_EQ(read, secondBlob);
	}

	TEST(PipelineCacheTest, WriteBlobKeepsNoTemporaryFileBehind)
	{
		ScopedTemporaryDirectory directory;
		const auto path = directory.path() / "pipeline.bin";
		const auto properties = makeProperties(0x10DE, 0x2484, 7);

		ASSERT_TRUE(evan::PipelineCache::writeBlob(path, makeBlob(properties)));

		for (const auto &entry:
			 std::filesystem::directory_iterator(directory.path())) {
			EXPECT_EQ(entry.path().filename(), path.filename());
		}
	}

	TEST(PipelineCacheTest, WriteBlobRejectsEmptyInput)
	{
		ScopedTemporaryDirectory directory;
		const auto path = directory.path() / "pipeline.bin";

		EXPECT_FALSE(evan::PipelineCache::writeBlob(path, {}));
		EXPECT_FALSE(std::filesystem::exists(path));
	}

	TEST(PipelineCacheTest, ReadBlobRejectsMissingFile)
	{
		ScopedTemporaryDirectory directory;
		std::vector<std::byte> blob;

		EXPECT_FALSE(evan::PipelineCache::readBlob(
			directory.path() / "missing.bin", blob));
		EXPECT_TRUE(blob.empty());
	}

	TEST(PipelineCacheTest, ReadBlobRejectsEmptyPath)
	{
		std::vector<std::byte> blob;

		EXPECT_FALSE(evan::PipelineCache::readBlob(
			std::filesystem::path(), blob));
	}

	TEST(PipelineCacheTest, UsesPlatformCacheRootByDefault)
	{
		ScopedEnvironmentVariable clearedOverride(kCacheDirVariable, "");
		ScopedEnvironmentVariable clearedSwitch(kDisableVariable, "");
		ScopedTemporaryDirectory root;
		const TestablePlatform platform(root.path());

		EXPECT_EQ(platform.getPipelineCachePath(),
				  root.path() / "evan" / "pipeline.bin");
	}

	TEST(PipelineCacheTest, HonorsCacheDirectoryOverride)
	{
		ScopedTemporaryDirectory directory;
		ScopedEnvironmentVariable override(kCacheDirVariable,
										   directory.path().string());
		ScopedEnvironmentVariable clearedSwitch(kDisableVariable, "");
		const TestablePlatform platform(directory.path() / "ignored");

		EXPECT_EQ(platform.getPipelineCachePath(),
				  directory.path() / "evan" / "pipeline.bin");
	}

	TEST(PipelineCacheTest, ReturnsNoPathWhenPersistenceIsDisabled)
	{
		ScopedEnvironmentVariable disabled(kDisableVariable, "1");
		const TestablePlatform platform("/somewhere");

		EXPECT_TRUE(platform.getPipelineCachePath().empty());
	}

	TEST(PipelineCacheTest, ReturnsNoPathWithoutWritableRoot)
	{
		ScopedEnvironmentVariable clearedOverride(kCacheDirVariable, "");
		ScopedEnvironmentVariable clearedSwitch(kDisableVariable, "");
		const TestablePlatform platform{std::filesystem::path{}};

		EXPECT_TRUE(platform.getPipelineCachePath().empty());
	}
}	 // namespace xider::tests
