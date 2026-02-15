cmake_minimum_required(VERSION 3.10)

project(evan
    VERSION 1.0.0
    DESCRIPTION "Evan"
    HOMEPAGE_URL "https://etib-corp.github.io/evan"
    LANGUAGES CXX
)

# Include common functions
include(cmake/CommonFunctions.cmake)

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

# Platform-specific configuration (must come after feature options)
if(WIN32)
    include(cmake/ConfigWindows.cmake)
elseif(APPLE)
    include(cmake/ConfigMacOS.cmake)
elseif(UNIX)
    include(cmake/ConfigLinux.cmake)
endif()

# Global GLM defines
add_compile_definitions(GLM_FORCE_RADIANS)
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

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

set(GLM_BUILD_TESTS OFF)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.2
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(utility
    GIT_REPOSITORY https://github.com/etib-corp/utility.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(utility)

find_package(Vulkan REQUIRED)

# Collect common source files
file(GLOB SOURCES_COMMON "${SOURCES_DIR}/common/*.cpp" "${SOURCES_DIR}/*.cpp")

if(BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED ${SOURCES_COMMON})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCES_COMMON})
endif()

# Link common dependencies
target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan glm utility)

# Configure for OpenXR
if(BUILD_FOR_OPENXR)
    message(STATUS "Building with OpenXR support")

    # Add OpenXR platform defines
    add_compile_definitions(XR_USE_GRAPHICS_API_VULKAN)
    # Android-specific defines and includes
    if(ANDROID)
        add_compile_definitions(XR_USE_PLATFORM_ANDROID)
        add_compile_definitions(VK_USE_PLATFORM_ANDROID_KHR)
        include_directories(${JAVA_INCLUDE_PATHS})
        find_library(log-lib log)
        find_library(android-lib android)
        find_library(jnigraphics-lib jnigraphics)
    endif()

    # Disable OpenXR tests and other unnecessary components
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_CONFORMANCE_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_API_LAYERS OFF CACHE BOOL "" FORCE)

    # Fetch OpenXR SDK
    FetchContent_Declare(OpenXR-SDK
        GIT_REPOSITORY https://github.com/KhronosGroup/OpenXR-SDK-Source.git
        GIT_TAG release-1.1.54
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(OpenXR-SDK)

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
        target_link_libraries(${PROJECT_NAME} PRIVATE OpenXR::openxr_loader ${log-lib} ${android-lib} ${jnigraphics-lib})
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
    target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
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
option(BUILD_DOCS "Build documentation" OFF)
option(BUILD_EXAMPLES "Build the examples" OFF)

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
