add_executable(GLFW-windows-base
    main.cpp
)

target_link_libraries(GLFW-windows-base PRIVATE
    evan
)

add_custom_command(TARGET GLFW-windows-base POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/examples/GLFW-windows-base/textures
    $<TARGET_FILE_DIR:GLFW-windows-base>/textures
)
