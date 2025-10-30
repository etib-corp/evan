cmake_minimum_required(VERSION 3.10)

project(evan
    VERSION 1.0.0
    DESCRIPTION "Evan"
    HOMEPAGE_URL "https://etib-corp.github.io/evan"
    LANGUAGES CXX
)

# Include common functions
include(cmake/CommonFunctions.cmake)

# Platform-specific configuration
if(WIN32)
    include(cmake/ConfigWindows.cmake)
elseif(APPLE)
    include(cmake/ConfigMacOS.cmake)
elseif(UNIX)
    include(cmake/ConfigLinux.cmake)
endif()

# Set global C++ standard and properties
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Feature options
option(BUILD_FOR_OPENXR "Build with OpenXR support" OFF)
option(BUILD_FOR_GLFW "Build with GLFW support" OFF)
option(BUILD_SHARED_LIBS "Build as shared library" OFF)

# Check that at least one build mode is selected
if(NOT BUILD_FOR_OPENXR AND NOT BUILD_FOR_GLFW)
    if(BUILD_DOCS)
        message(STATUS "No backend selected; proceeding because BUILD_DOCS=ON (docs target only).")
    else()
        message(FATAL_ERROR "At least one of BUILD_FOR_OPENXR or BUILD_FOR_GLFW must be ON.")
    endif()
endif()

# Set the files directories
set(HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/headers")
set(SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sources")

# Add the headers directories
include_directories(${HEADERS_DIR})

# FetchContent module
include(FetchContent)

# Fetch Vulkan-Headers
FetchContent_Declare(Vulkan-Headers
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG v1.4.329
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(Vulkan-Headers)

set(GLM_BUILD_TESTS OFF)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.2
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glm)

# Collect common source files
file(GLOB SOURCES_COMMON "${SOURCES_DIR}/common/*.cpp" "${SOURCES_DIR}/*.cpp")

if(BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED ${SOURCES_COMMON})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCES_COMMON})
endif()

# Link common dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan-Headers glm)

# Configure for OpenXR
if(BUILD_FOR_OPENXR)
    message(STATUS "Building with OpenXR support")

    # Disable OpenXR tests and other unnecessary components
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_CONFORMANCE_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_API_LAYERS OFF CACHE BOOL "" FORCE)

    # Fetch OpenXR SDK
    FetchContent_Declare(OpenXR-SDK
        GIT_REPOSITORY https://github.com/KhronosGroup/OpenXR-SDK.git
        GIT_TAG release-1.1.53
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(OpenXR-SDK)

    # Find JNI only if building for Android
    if(ANDROID)
        find_package(JNI REQUIRED)
    endif()

    # Add OpenXR compile definition
    add_compile_definitions(__OPENXR__)

    # Collect OpenXR-specific source files
    file(GLOB_RECURSE SOURCES_XR "${SOURCES_DIR}/openxr/*.cpp")
    
    # Exclude Android-specific files on non-Android platforms
    if(NOT ANDROID)
        list(FILTER SOURCES_XR EXCLUDE REGEX ".*AndroidPlatform\\.cpp$")
    endif()

    # Create library with OpenXR support
    target_sources(${PROJECT_NAME} PRIVATE ${SOURCES_XR})

    # Link OpenXR dependencies
    if(ANDROID)
        target_link_libraries(${PROJECT_NAME} PRIVATE OpenXR::openxr_loader JNI::JNI)
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE OpenXR::openxr_loader)
    endif()
endif()

# Configure for GLFW
if(BUILD_FOR_GLFW)
    message(STATUS "Building with GLFW support")

    # Fetch GLFW for window management
    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(glfw)

    # Add Vulkan compile definition
    add_compile_definitions(__GLFW__)

    # Collect Vulkan-specific source files
    file(GLOB_RECURSE GLFW_SOURCES "${SOURCES_DIR}/glfw/*.cpp")

    # Create library with Vulkan support
    target_sources(${PROJECT_NAME} PRIVATE ${GLFW_SOURCES})

    # Link Vulkan dependencies
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
endif()

# Set common target properties
set_evan_target_properties(${PROJECT_NAME})

# Configure public interface
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${HEADERS_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Optional components
option(BUILD_TESTING "Build the tests" ON)
option(BUILD_DOCS "Build documentation" OFF)
option(BUILD_EXAMPLES "Build the examples" OFF)

if(BUILD_TESTING)
    message(STATUS "Building tests...")
    add_subdirectory(tests)
else()
    message(STATUS "Skipping tests...")
endif()

if(BUILD_DOCS)
    message(STATUS "Building docs...")
    add_subdirectory(docs)
else()
    message(STATUS "Skipping docs...")
endif()

if(BUILD_EXAMPLES)
    message(STATUS "Building examples...")
    add_subdirectory(examples)
else()
    message(STATUS "Skipping examples...")
endif()
