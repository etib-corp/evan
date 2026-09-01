add_executable(GLFW-macos-base
    main.cpp
)

target_link_libraries(GLFW-macos-base PRIVATE
    evan
)

add_custom_command(TARGET GLFW-macos-base POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/examples/GLFW-macos-base/textures
    $<TARGET_FILE_DIR:GLFW-macos-base>/textures
)