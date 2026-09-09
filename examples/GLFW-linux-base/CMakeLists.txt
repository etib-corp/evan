add_executable(GLFW-linux-base
    main.cpp
)

target_link_libraries(GLFW-linux-base PRIVATE
    evan
)

add_custom_command(TARGET GLFW-linux-base POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/examples/GLFW-linux-base/textures
    $<TARGET_FILE_DIR:GLFW-linux-base>/textures
)
