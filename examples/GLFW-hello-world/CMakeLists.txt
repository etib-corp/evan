cmake_minimum_required(VERSION 3.10)

project(Hello-World)

# Set global C++ standard and properties
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the files directories
set(HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/headers")
set(SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sources")

# Add the headers directories
include_directories(${HEADERS_DIR})

# FetchContent module
include(FetchContent)

set(BUILD_FOR_GLFW TRUE)

# Collect common source files
file(GLOB SOURCES "${SOURCES_DIR}/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Link common dependencies
target_link_libraries(${PROJECT_NAME} PUBLIC evan)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)