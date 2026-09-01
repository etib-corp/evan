/*
** ETIB PROJECT, 2026
** evan
** File description:
** ViewSet
*/

#pragma once

#include "evan/EvanPlatform.hpp"

#include <utility/graphic/view.hpp>

#include <cstddef>
#include <vector>

namespace evan
{
	/**
	 * @brief Describes a set of views to render, independent of the swapchain
	 * image sets.
	 *
	 * A ViewSet is an ordered list of views (camera poses + projections). Each
	 * view is associated with the index of the swapchain image set it renders
	 * into. This decouples the concept of "how many views must be rendered"
	 * from "how many swapchains / images exist":
	 *
	 * - Desktop: one view rendering into swapchain set 0.
	 * - OpenXR: one view per eye, each rendering into its own swapchain set.
	 * - Desktop multi-view (future): several views sharing swapchain set 0.
	 *
	 * The Renderer iterates over a ViewSet to know which view matrices to
	 * upload (one uniform buffer update per view) and which swapchain image to
	 * render each view into.
	 */
	class ViewSet
	{
		public:
		/**
		 * @brief A single view together with the swapchain it targets.
		 */
		struct View {
			/**
			 * @brief The view state: pose, field of view, clipping planes and
			 * viewport. Used to build the view and projection matrices.
			 */
			utility::graphic::ViewF view;

			/**
			 * @brief Index of the swapchain image set this view renders into.
			 */
			std::size_t swapchainIndex = 0;
		};

		/**
		 * @brief Appends a view bound to the given swapchain.
		 *
		 * @param view The view state to append.
		 * @param swapchainIndex Index of the swapchain image set to target.
		 */
		void addView(const utility::graphic::ViewF &view,
					 std::size_t swapchainIndex);
		/**
		 * @brief Sets the view state at the given index, growing the set if
		 * necessary.
		 *
		 * The existing swapchain index is preserved when the view already
		 * exists; new entries default to swapchain index 0.
		 *
		 * @param index Index of the view to set.
		 * @param view The view state to store.
		 */
		void setView(std::size_t index, const utility::graphic::ViewF &view);

		/**
		 * @brief Sets the swapchain index of the view at the given index,
		 * growing the set if necessary.
		 *
		 * @param index Index of the view to update.
		 * @param swapchainIndex Index of the swapchain image set to target.
		 */
		void setSwapchainIndex(std::size_t index, std::size_t swapchainIndex);

		/**
		 * @brief Resizes the view set to contain exactly @p count views.
		 *
		 * @param count The new number of views.
		 */
		void resize(std::size_t count);

		/**
		 * @brief Removes all views from the set.
		 */
		void clear();

		/**
		 * @return The number of views in the set.
		 */
		std::size_t size() const;

		/**
		 * @return True when the set contains no view.
		 */
		bool empty() const;

		/**
		 * @brief Accesses the view at the given index (unchecked).
		 *
		 * @param index Index of the view to access.
		 * @return The view entry.
		 */
		View &operator[](std::size_t index)
		{
			return _views[index];
		}

		/**
		 * @brief Accesses the view at the given index (unchecked, const).
		 *
		 * @param index Index of the view to access.
		 * @return The view entry.
		 */
		const View &operator[](std::size_t index) const
		{
			return _views[index];
		}

		/**
		 * @brief Accesses the view at the given index with bounds checking.
		 *
		 * @param index Index of the view to access.
		 * @return The view entry.
		 */
		View &at(std::size_t index)
		{
			return _views.at(index);
		}

		/**
		 * @brief Accesses the view at the given index with bounds checking
		 * (const).
		 *
		 * @param index Index of the view to access.
		 * @return The view entry.
		 */
		const View &at(std::size_t index) const
		{
			return _views.at(index);
		}

		/**
		 * @brief Retrieves the view state for the given index.
		 *
		 * @param index Index of the view.
		 * @return The view state.
		 */
		utility::graphic::ViewF getView(std::size_t index) const;

		/**
		 * @brief Retrieves the projection matrix for the given index.
		 *
		 * @param index Index of the view.
		 * @return The projection matrix.
		 */
		glm::mat4 getProjection(std::size_t index) const;

		private:
		/**
		 * @brief The ordered list of views.
		 */
		std::vector<View> _views;
	};
}	 // namespace evan
