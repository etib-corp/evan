/*
** ETIB PROJECT, 2026
** evan
** File description:
** IPlatform
*/

#include "evan/IPlatform.hpp"

#include <cstdlib>
#include <string>

namespace
{
	/**
	 * @brief Environment variable overriding the pipeline cache directory.
	 */
	constexpr const char *kCacheDirVariable = "EVAN_PIPELINE_CACHE_DIR";

	/**
	 * @brief Environment variable disabling pipeline cache persistence.
	 */
	constexpr const char *kDisableVariable = "EVAN_PIPELINE_CACHE_DISABLE";

	/**
	 * @brief Directory, inside the platform cache root, holding Evan blobs.
	 */
	constexpr const char *kCacheSubDirectory = "evan";

	/**
	 * @brief Name of the persisted pipeline cache blob.
	 */
	constexpr const char *kCacheFileName = "pipeline.bin";

	/**
	 * @brief Reads an environment variable.
	 *
	 * @param name Name of the variable to read.
	 * @return The value of the variable, or an empty string when it is not
	 * set.
	 */
	std::string getEnvironment(const char *name)
	{
		const char *value = std::getenv(name);
		return value != nullptr ? std::string(value) : std::string();
	}

	/**
	 * @brief Tells whether an environment variable holds an enabled value.
	 *
	 * Matches the convention already used by the renderer debug switches: the
	 * variable must be set to a non-empty value that is not `0`.
	 *
	 * @param name Name of the variable to read.
	 * @return True when the variable enables the option.
	 */
	bool isEnvironmentEnabled(const char *name)
	{
		const char *value = std::getenv(name);
		return value != nullptr && value[0] != '\0' && value[0] != '0';
	}
}	 // namespace

std::filesystem::path evan::IPlatform::getPipelineCachePath() const
{
	if (isEnvironmentEnabled(kDisableVariable)) {
		this->getLogger().info()
			<< "Pipeline cache persistence disabled through "
			<< kDisableVariable << ".";
		return {};
	}

	const std::string overridden = getEnvironment(kCacheDirVariable);
	const std::filesystem::path root =
		!overridden.empty() ? std::filesystem::path(overridden)
							: this->getDefaultCacheRoot();

	if (root.empty()) {
		this->getLogger().info()
			<< "No writable cache directory found; pipeline cache "
			   "persistence disabled.";
		return {};
	}

	return root / kCacheSubDirectory / kCacheFileName;
}
