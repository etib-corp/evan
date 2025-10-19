cmake_minimum_required(VERSION 3.10)

project(evan
    VERSION 1.0.0
    DESCRIPTION "Evan - Graphics Engine Library"
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
option(ENABLE_XR "Enable XR support" OFF)
option(BUILD_SHARED_LIBS "Build as shared library" OFF)

# Set the files directories
set(HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/headers")
set(SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sources")

# Add the headers directories
include_directories(${HEADERS_DIR})

include(FetchContent)

# Fetch GLM (OpenGL Mathematics)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glm)

# Collect common source files
file(GLOB_RECURSE SOURCES_COMMON "${SOURCES_DIR}/common/*.cpp" "${SOURCES_DIR}/Version.cpp")

# Platform-specific library paths (macOS uses Homebrew)
if(APPLE)
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
endif()

# Configure XR or Vulkan mode
if(ENABLE_XR)
    message(STATUS "Building with XR support enabled")
    
    # XR requires C++20
    if(CMAKE_CXX_STANDARD LESS 20)
        set(CMAKE_CXX_STANDARD 20)
    endif()
    
    # Add XR compile definition
    add_compile_definitions(__XR__)

    # Fetch OpenXR SDK
    FetchContent_Declare(OpenXR-SDK
        GIT_REPOSITORY https://github.com/KhronosGroup/OpenXR-SDK.git
        GIT_TAG release-1.0.33  # Must match the Meta Quest loader OpenXR version
        # https://developer.oculus.com/downloads/package/oculus-openxr-mobile-sdk#current-openxr-version
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(OpenXR-SDK)

    # Collect XR-specific source files
    file(GLOB_RECURSE SOURCES_XR "${SOURCES_DIR}/xr/*.cpp")
    
    # Create library with XR support
    if(BUILD_SHARED_LIBS)
        add_library(${PROJECT_NAME} SHARED ${SOURCES_COMMON} ${SOURCES_XR})
    else()
        add_library(${PROJECT_NAME} STATIC ${SOURCES_COMMON} ${SOURCES_XR})
    endif()

    # Link XR dependencies
    target_link_libraries(${PROJECT_NAME} PRIVATE openxr_loader vulkan glm::glm)
    
else()
    message(STATUS "Building with Vulkan support (standard mode)")
    
    # Add Vulkan compile definition
    add_compile_definitions(__VK__)

    # Fetch GLFW for window management
    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(glfw)

    # Collect Vulkan-specific source files
    file(GLOB_RECURSE SOURCES_VK "${SOURCES_DIR}/vk/*.cpp")

    # Create library with Vulkan support
    if(BUILD_SHARED_LIBS)
        add_library(${PROJECT_NAME} SHARED ${SOURCES_COMMON} ${SOURCES_VK})
    else()
        add_library(${PROJECT_NAME} STATIC ${SOURCES_COMMON} ${SOURCES_VK})
    endif()

    # Link Vulkan dependencies
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm::glm)
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
