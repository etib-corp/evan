/*
** ETIB PROJECT, 2026
** evan
** File description:
** GlfwContext
*/

#include "evan/glfw/GlfwContext.hpp"

#include "evan/EvanPlatform.hpp"

#include <stdexcept>

namespace
{
	/**
	 * @brief Process-global GLFW state.
	 *
	 * The constructor initializes GLFW and the destructor terminates it. The
	 * lifecycle is reference-counted through std::shared_ptr so that
	 * `glfwTerminate()` is only invoked when the last guard is released.
	 */
	struct GlfwState {
		GlfwState()
		{
			if (!glfwInit()) {
				throw std::runtime_error("Failed to initialize GLFW");
			}
		}

		~GlfwState()
		{
			glfwTerminate();
		}
	};

	std::shared_ptr<void> acquireGlfw()
	{
		static std::weak_ptr<void> weak;

		if (auto guard = weak.lock()) {
			return guard;
		}

		auto guard = std::shared_ptr<void>(new GlfwState());
		weak	   = guard;
		return guard;
	}
}	 // namespace

evan::glfw::GlfwContext::GlfwContext()
	: _state(acquireGlfw())
{
}
