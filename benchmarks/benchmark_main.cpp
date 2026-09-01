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

#include <benchmark/benchmark.h>

#include <memory>

#include <evan/Scene.hpp>

namespace
{

	// Scene::addObject/removeObject are CPU-side map operations that do not
	// require a live Vulkan device, so they are safe to benchmark directly.

	void BM_SceneAddObject(benchmark::State &state)
	{
		evan::Scene scene;
		for (auto _: state) {
			for (std::uint32_t i = 0; i < state.range(0); ++i) {
				scene.addObject(i, nullptr);
			}
			state.PauseTiming();
			for (std::uint32_t i = 0; i < state.range(0); ++i) {
				scene.removeObject(i);
			}
			state.ResumeTiming();
		}
	}
	BENCHMARK(BM_SceneAddObject)->Range(8, 8 << 10);

	void BM_SceneRemoveObject(benchmark::State &state)
	{
		evan::Scene scene;
		for (std::uint32_t i = 0; i < state.range(0); ++i) {
			scene.addObject(i, nullptr);
		}
		for (auto _: state) {
			for (std::uint32_t i = 0; i < state.range(0); ++i) {
				benchmark::DoNotOptimize(scene.removeObject(i));
			}
		}
	}
	BENCHMARK(BM_SceneRemoveObject)->Range(8, 8 << 10);

}	 // namespace

BENCHMARK_MAIN();
