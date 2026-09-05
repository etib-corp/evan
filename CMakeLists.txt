cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

project(evan
    VERSION 1.0.0
    DESCRIPTION "Evan"
    HOMEPAGE_URL "https://etib-corp.github.io/evan"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 01. Policy & standard setup

# 02. Options & configuration
option(BUILD_FOR_OPENXR "Build with OpenXR support" OFF)
option(BUILD_FOR_GLFW "Build with GLFW support" OFF)
option(BUILD_FOR_ANDROID "Build for Android" OFF)
option(BUILD_FOR_LINUX "Build for Linux" OFF)
option(BUILD_FOR_WINDOWS "Build for Windows" OFF)
option(BUILD_FOR_MACOS "Build for macOS" OFF)
option(BUILD_DOCS "Build Doxygen documentation" OFF)
option(BUILD_DOCS_ONLY "Configure only the documentation target" OFF)
option(BUILD_ALL_DOCS "Build documentation for all libraries" OFF)
option(BUILD_BENCHMARKS "Build the benchmark harness" OFF)
option(DEV_MODE "Enable development mode" OFF)

if(BUILD_DOCS_ONLY)
    set(BUILD_DOCS ON)
    set(BUILD_TESTING OFF)
endif()

set(_SKIP_PLATFORM_VALIDATION FALSE)
if(BUILD_DOCS_ONLY)
    set(_SKIP_PLATFORM_VALIDATION TRUE)
endif()

set(PLATFORM_OPTION_COUNT 0)
if(BUILD_FOR_ANDROID)
    math(EXPR PLATFORM_OPTION_COUNT "${PLATFORM_OPTION_COUNT} + 1")
endif()
if(BUILD_FOR_LINUX)
    math(EXPR PLATFORM_OPTION_COUNT "${PLATFORM_OPTION_COUNT} + 1")
endif()
if(BUILD_FOR_WINDOWS)
    math(EXPR PLATFORM_OPTION_COUNT "${PLATFORM_OPTION_COUNT} + 1")
endif()
if(BUILD_FOR_MACOS)
    math(EXPR PLATFORM_OPTION_COUNT "${PLATFORM_OPTION_COUNT} + 1")
endif()

if(NOT _SKIP_PLATFORM_VALIDATION)
    if(NOT PLATFORM_OPTION_COUNT EQUAL 1)
        message(FATAL_ERROR "Invalid platform configuration: enable exactly one platform option (BUILD_FOR_ANDROID, BUILD_FOR_LINUX, BUILD_FOR_WINDOWS, BUILD_FOR_MACOS).")
    endif()
endif()

set(BACKEND_OPTION_COUNT 0)
if(BUILD_FOR_OPENXR)
    math(EXPR BACKEND_OPTION_COUNT "${BACKEND_OPTION_COUNT} + 1")
endif()
if(BUILD_FOR_GLFW)
    math(EXPR BACKEND_OPTION_COUNT "${BACKEND_OPTION_COUNT} + 1")
endif()

if(NOT _SKIP_PLATFORM_VALIDATION)
    if(NOT BACKEND_OPTION_COUNT EQUAL 1)
        message(FATAL_ERROR "Invalid backend configuration: enable exactly one backend option (BUILD_FOR_OPENXR or BUILD_FOR_GLFW).")
    endif()
endif()

if(BUILD_FOR_LINUX)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConfigLinux.cmake)
elseif(BUILD_FOR_MACOS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConfigMacOS.cmake)
elseif(BUILD_FOR_WINDOWS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConfigWindows.cmake)
elseif(BUILD_FOR_ANDROID)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConfigAndroid.cmake)
endif()

# 03. Global properties

# Paths
set(HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/headers")
set(SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sources")

if(NOT BUILD_DOCS_ONLY)
    # Dependencies
    include(FetchContent)

    set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.2
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(glm)

    FetchContent_Declare(stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(stb)

    find_package(Vulkan REQUIRED)

    FetchContent_Declare(utility
        GIT_REPOSITORY https://github.com/etib-corp/utility.git
        GIT_TAG main
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    # Prefer a local utility checkout (../utility) for development so that
    # local fixes are picked up without pushing to GitHub first.
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../utility/CMakeLists.txt")
        set(FETCHCONTENT_SOURCE_DIR_UTILITY
            "${CMAKE_CURRENT_SOURCE_DIR}/../utility" CACHE PATH "" FORCE)
    endif()
    FetchContent_MakeAvailable(utility)

    # 04. Library definition
    include(cmake/Sources.cmake)

    if(BUILD_SHARED_LIBS)
        add_library(${PROJECT_NAME} SHARED)
    else()
        add_library(${PROJECT_NAME} STATIC)
    endif()

    target_sources(${PROJECT_NAME}
        PRIVATE
            ${EVAN_SOURCES_COMMON}
    )

    # 05. Include directories
    target_include_directories(${PROJECT_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${HEADERS_DIR}>
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${stb_SOURCE_DIR}>
    )

    # 06. Compile definitions & options
    target_compile_definitions(${PROJECT_NAME}
        PUBLIC
            GLM_FORCE_RADIANS
            GLM_FORCE_DEPTH_ZERO_TO_ONE
            GLM_ENABLE_EXPERIMENTAL
    )

    # 07. Link dependencies
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            Vulkan::Vulkan
            glm
            utility
    )

    # 08. Backend-specific configuration

    # OpenXR flags
    if(BUILD_FOR_OPENXR)
        message(STATUS "Building with OpenXR support")
        target_compile_definitions(${PROJECT_NAME}
            PUBLIC
                XR_USE_GRAPHICS_API_VULKAN
                __OPENXR__
        )
    endif()

    # OpenXR Android flags
    if(BUILD_FOR_OPENXR AND BUILD_FOR_ANDROID)
        target_compile_definitions(${PROJECT_NAME}
            PUBLIC
                XR_USE_PLATFORM_ANDROID
                VK_USE_PLATFORM_ANDROID_KHR
        )

        if(JAVA_INCLUDE_PATHS)
            target_include_directories(${PROJECT_NAME} PRIVATE ${JAVA_INCLUDE_PATHS})
        endif()

        find_library(log-lib log)
        find_library(android-lib android)
        find_library(jnigraphics-lib jnigraphics)
    endif()

    # Android system libraries are only available when an Android NDK
    # toolchain is in use. Guard each link so a missing NDK does not hard-fail
    # configuration (e.g. host builds without the NDK installed).
    if(BUILD_FOR_OPENXR AND BUILD_FOR_ANDROID)
        if(log-lib)
            target_link_libraries(${PROJECT_NAME} PRIVATE ${log-lib})
        endif()
        if(android-lib)
            target_link_libraries(${PROJECT_NAME} PRIVATE ${android-lib})
        endif()
        if(jnigraphics-lib)
            target_link_libraries(${PROJECT_NAME} PRIVATE ${jnigraphics-lib})
        endif()
    endif()

    # OpenXR dependency options
    if(BUILD_FOR_OPENXR)
        set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(BUILD_CONFORMANCE_TESTS OFF CACHE BOOL "" FORCE)
        set(BUILD_API_LAYERS OFF CACHE BOOL "" FORCE)
        set(BUILD_WITH_SYSTEM_JSONCPP OFF CACHE BOOL "Use bundled jsoncpp for OpenXR SDK" FORCE)
    endif()

    # OpenXR fetch
    if(BUILD_FOR_OPENXR)
        FetchContent_Declare(OpenXR-SDK
            GIT_REPOSITORY https://github.com/KhronosGroup/OpenXR-SDK-Source.git
            GIT_TAG release-1.1.54
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(OpenXR-SDK)
    endif()

    # OpenXR sources
    if(BUILD_FOR_OPENXR)
        target_sources(${PROJECT_NAME}
            PRIVATE
                ${EVAN_SOURCES_OPENXR}
        )

        if(BUILD_FOR_ANDROID)
            target_sources(${PROJECT_NAME}
                PRIVATE
                    ${EVAN_SOURCES_OPENXR_ANDROID}
            )
        endif()
    endif()

    # OpenXR links
    if(BUILD_FOR_OPENXR)
        target_link_libraries(${PROJECT_NAME}
            PUBLIC
                OpenXR::openxr_loader
        )
    endif()

    # GLFW fetch
    if(BUILD_FOR_GLFW)
        message(STATUS "Building with GLFW support")

        FetchContent_Declare(glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(glfw)
    endif()

    # GLFW definitions
    if(BUILD_FOR_GLFW)
        target_compile_definitions(${PROJECT_NAME}
            PUBLIC
                __GLFW__
        )
    endif()

    # GLFW sources
    if(BUILD_FOR_GLFW)
        target_sources(${PROJECT_NAME}
            PRIVATE
                ${EVAN_SOURCES_GLFW}
        )
    endif()

    # GLFW links
    if(BUILD_FOR_GLFW)
        target_link_libraries(${PROJECT_NAME}
            PUBLIC
                glfw
        )
    endif()

    if(DEV_MODE)
        target_compile_definitions(
            ${PROJECT_NAME}
            PRIVATE
            DEV
        )
    endif()

    # 08b. Packaging & consumability
    include(GNUInstallDirs)

    install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(DIRECTORY ${HEADERS_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        cmake/${PROJECT_NAME}Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    # CPack packaging
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
    set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
    set(CPACK_PACKAGE_VENDOR "ETIB Corporation")
    set(CPACK_PACKAGE_CONTACT "clement-antoine.xavier@epitech.eu")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    include(CPack)
endif()

# 09. Documentations

if(BUILD_ALL_DOCS OR BUILD_DOCS_ONLY)
    message(STATUS "Building documentation...")
    include(cmake/Docs.cmake)
    add_subdirectory(docs)
else()
    message(STATUS "Skipping documentation...")
endif()

# 10. Tests

if(BUILD_TESTING)
    message(STATUS "Building tests...")
    add_subdirectory(tests)
else()
    message(STATUS "Skipping tests...")
endif()

# 11. Benchmarks

if(BUILD_BENCHMARKS)
    message(STATUS "Building benchmarks...")
    add_subdirectory(benchmarks)
endif()

# 12. Examples

add_subdirectory(examples)
