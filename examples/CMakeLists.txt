# Evan examples

option(BUILD_EXAMPLES "Build the runnable examples" OFF)

if(BUILD_EXAMPLES)
    add_subdirectory(GLFW-macos-base)
    add_subdirectory(GLFW-linux-base)
    add_subdirectory(GLFW-windows-base)

    # P3 proof-of-concept workloads (docs/DESIGN_P3_OPTIMIZATIONS.md §4).
    add_subdirectory(GLFW-macos-poc)
    add_subdirectory(GLFW-linux-poc)

    if(BUILD_OPENXR)
        add_subdirectory(OpenXR-android-base)
        add_subdirectory(OpenXR-linux-base)
    endif()
endif()
