add_executable(GLFW-stress
    main.cpp
)

target_link_libraries(GLFW-stress PRIVATE
    evan
)

add_custom_command(TARGET GLFW-stress POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/textures
    $<TARGET_FILE_DIR:GLFW-stress>/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:GLFW-stress>/shaders
)
