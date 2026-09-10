# Evan examples

option(BUILD_EXAMPLES "Build the runnable examples" OFF)

if(BUILD_EXAMPLES)
    add_subdirectory(GLFW-macos-base)
    add_subdirectory(GLFW-linux-base)
    add_subdirectory(GLFW-windows-base)
    add_subdirectory(OpenXR-linux-base)
endif()
