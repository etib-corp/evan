# Enable testing
enable_testing()

# Set test directories
set(TEST_HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/headers")
set(TEST_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sources")

# Find all source files in the test sources directory
file(GLOB_RECURSE TEST_SOURCES "${TEST_SOURCES_DIR}/*.cpp")

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
# Prevent GoogleTest from overriding our compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Create test executable
add_executable(test_${PROJECT_NAME} ${TEST_SOURCES})

# Set common target properties
set_xider_target_properties(test_${PROJECT_NAME})

# Configure include directories
target_include_directories(test_${PROJECT_NAME} PRIVATE
    ${TEST_HEADERS_DIR}
)

# Link libraries
target_link_libraries(test_${PROJECT_NAME} PRIVATE
    xider
    gtest_main
)

# Discover and register tests
include(GoogleTest)
gtest_discover_tests(test_${PROJECT_NAME})
