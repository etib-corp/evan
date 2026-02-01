/*
** ETIB PROJECT, 2026
** evan
** File description:
** App
*/

#include "App.hpp"

evan::App::App(const AppProperties &properties)
{
}

evan::App::~App()
{
}

std::shared_ptr<evan::Primitive> &evan::App::display(std::shared_ptr<Primitive> primitive)
{
    _primitives.push_back(primitive);
    return _primitives.back();
}

void evan::App::drawFrame(float frameRate)
{
    for (const auto &primitive : _primitives) {
    }
}

void evan::App::run()
{
    // while(shouldClose()) {
    //     _software->pollEvent();
    //     drawFrame(60.0f)
    // }
}
