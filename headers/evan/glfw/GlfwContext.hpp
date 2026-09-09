/*
** ETIB PROJECT, 2026
** evan
** File description:
** GlfwContext
*/

#pragma once

#include <memory>

namespace evan::glfw
{
	/**
	 * @brief Reference-counted GLFW lifecycle guard.
	 *
	 * GLFW is process-global state: `glfwInit()` and `glfwTerminate()` must
	 * not be paired per platform instance, otherwise destroying one platform
	 * tears down GLFW for every other live platform. This guard acquires the
	 * library when the first instance is created and releases it when the
	 * last instance is destroyed.
	 *
	 * The type is copyable and movable; copies share the same underlying
	 * reference count (backed by std::shared_ptr).
	 */
	class GlfwContext
	{
		public:
		/**
		 * @brief Acquires the GLFW library.
		 *
		 * Increments the global reference count and calls `glfwInit()` on the
		 * first acquisition.
		 *
		 * @throws std::runtime_error If `glfwInit()` fails.
		 */
		GlfwContext();

		/**
		 * @brief Releases the GLFW library.
		 *
		 * Decrements the global reference count and calls `glfwTerminate()`
		 * when the last guard is destroyed.
		 */
		~GlfwContext() = default;

		GlfwContext(const GlfwContext &)				= default;
		GlfwContext(GlfwContext &&) noexcept			= default;
		GlfwContext &operator=(const GlfwContext &)		= default;
		GlfwContext &operator=(GlfwContext &&) noexcept = default;

		private:
		/**
		 * @brief Shared, type-erased state keeping GLFW alive.
		 */
		std::shared_ptr<void> _state;
	};
}	 // namespace evan::glfw
