add_executable(GLFW-linux-poc
    main.cpp
)

target_link_libraries(GLFW-linux-poc PRIVATE
    evan
)

add_custom_command(TARGET GLFW-linux-poc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/examples/GLFW-linux-poc/textures
    $<TARGET_FILE_DIR:GLFW-linux-poc>/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/examples/GLFW-linux-poc/shaders
    $<TARGET_FILE_DIR:GLFW-linux-poc>/shaders
)
