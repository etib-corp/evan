# Benchmark harness for Evan.
#
# Built only when BUILD_BENCHMARKS=ON. Uses Google Benchmark to measure the key
# hot paths: render-object creation, mesh/material setup, and the frame loop.

include(FetchContent)
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.1
)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(evan_benchmarks
    benchmark_main.cpp
)

target_link_libraries(evan_benchmarks PRIVATE
    evan
    benchmark::benchmark
)

target_include_directories(evan_benchmarks PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
