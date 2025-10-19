# Automatically discover and add all example subdirectories
file(GLOB EXAMPLE_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(dir ${EXAMPLE_DIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir} AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/CMakeLists.txt)
        message(STATUS "Found example directory: ${dir}")
        add_subdirectory(${dir})
    endif()
endforeach()
