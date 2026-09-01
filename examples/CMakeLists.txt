# Evan examples

option(BUILD_EXAMPLES "Build the runnable examples" OFF)

if(BUILD_EXAMPLES)
    add_subdirectory(scene_objects)
    add_subdirectory(GLFW-macos-base)
endif()
