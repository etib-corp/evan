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

#include <cstdint>
#include <iostream>

#include <evan/Scene.hpp>

int main(void)
{
	evan::Scene scene;

	// Add renderable objects to the scene. In a real application these would
	// be RenderObject instances backed by GPU meshes; here we demonstrate the
	// CPU-side object-management API.
	for (std::uint32_t i = 0; i < 10; ++i) {
		scene.addObject(i, nullptr);
	}

	std::cout << "Added 10 objects to the scene.\n";

	// Remove a subset of objects.
	for (std::uint32_t i = 0; i < 5; ++i) {
		scene.removeObject(i);
	}

	std::cout << "Removed 5 objects from the scene.\n";
	return 0;
}
