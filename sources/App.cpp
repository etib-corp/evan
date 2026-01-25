/*
** ETIB PROJECT, 2026
** evan
** File description:
** App
*/

#include "App.hpp"

evan::App()
{
}

evan::App::~App()
{
}

std::shared_ptr<Primitive> &evan::App::display(std::shared_ptr<Primitive> primitive)
{
    _primitives.push_back(primitive);
    return _primitives.back();
}

void evan::App::drawFrame(float frameRate)
{
    for (const auto &primitive : _primitives)
    {
        // Draw each primitive
    }
}
