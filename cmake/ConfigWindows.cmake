message(STATUS "Windows configuration loaded")

# Windows platform defines
add_compile_definitions(NOMINMAX)
if(BUILD_FOR_GLFW)
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)
    add_compile_definitions(GLFW_EXPOSE_NATIVE_WIN32)
endif()
if(BUILD_FOR_OPENXR)
    add_compile_definitions(XR_USE_PLATFORM_WIN32)
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)
endif()

# Set Windows-specific configuration
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

# Ensure we use the DLL runtime
foreach(flag_var
    CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG)
    string(REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
endforeach()

# Set MSVC-specific compiler flags
if(MSVC)
    # Enable modern C++ features and warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive-")

    # Set debug flags
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")

    # Set release flags
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")

    # Disable specific warnings that are too noisy
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")  # Disable deprecated warnings
endif()
