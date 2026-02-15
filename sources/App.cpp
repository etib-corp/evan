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

void evan::App::display(std::shared_ptr<Primitive> primitive)
{
    // Should add the given primitive to the current scene's list of primitives to be rendered in the next frame.
}

void evan::App::drawFrame(float frameRate)
{
    // Should get the current scene's primitives, render them to vertices, and then draw the frame using the software's graphical context.
}

void evan::App::run()
{
    while(_software->shouldClose() == false) {
        _software->pollEvents();
        drawFrame(60.0f);
    }
}
