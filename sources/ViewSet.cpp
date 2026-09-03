/*
** ETIB PROJECT, 2026
** evan
** File description:
** ViewSet
*/

#include "evan/ViewSet.hpp"

void evan::ViewSet::addView(const utility::graphic::ViewF &view,
							std::size_t swapchainIndex)
{
	_views.push_back(View { view, swapchainIndex });
}

void evan::ViewSet::setView(std::size_t index,
							const utility::graphic::ViewF &view)
{
	if (index >= _views.size()) {
		_views.resize(index + 1);
	}
	_views[index].view = view;
}

void evan::ViewSet::setSwapchainIndex(std::size_t index,
									  std::size_t swapchainIndex)
{
	if (index >= _views.size()) {
		_views.resize(index + 1);
	}
	_views[index].swapchainIndex = swapchainIndex;
}

void evan::ViewSet::resize(std::size_t count)
{
	_views.resize(count);
}

void evan::ViewSet::clear()
{
	_views.clear();
}

std::size_t evan::ViewSet::size() const
{
	return _views.size();
}

bool evan::ViewSet::empty() const
{
	return _views.empty();
}

utility::graphic::ViewF evan::ViewSet::getView(std::size_t index) const
{
	return _views.at(index).view;
}

glm::mat4 evan::ViewSet::getProjection(std::size_t index) const
{
	return _views.at(index).view.getProjectionMatrix();
}
