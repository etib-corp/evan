add_executable(GLFW-macos-poc
    main.cpp
)

target_link_libraries(GLFW-macos-poc PRIVATE
    evan
)

add_custom_command(TARGET GLFW-macos-poc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/examples/GLFW-macos-poc/textures
    $<TARGET_FILE_DIR:GLFW-macos-poc>/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/examples/GLFW-macos-poc/shaders
    $<TARGET_FILE_DIR:GLFW-macos-poc>/shaders
)
