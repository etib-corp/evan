message(STATUS "Linux configuration loaded")

# Linux platform defines
if(BUILD_FOR_OPENXR)
    add_compile_definitions(XR_USE_PLATFORM_XLIB)
    add_compile_definitions(VK_USE_PLATFORM_XLIB_KHR)
endif()

# Set Linux-specific compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Set debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Set release flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Enable position independent code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
