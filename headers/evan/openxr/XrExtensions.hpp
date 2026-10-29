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
	 * @brief OpenXR instance-extension discovery and pure capability queries.
	 */
	class XrExtensions
	{
		public:
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
	};
}	 // namespace evan
