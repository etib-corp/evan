function(add_project_docs)
    # Find Doxygen package
    find_package(Doxygen REQUIRED)
    if(NOT Doxygen_FOUND)
        message(FATAL_ERROR "Doxygen not found. Documentation will not be built. Install doxygen and try again.")
    endif()

    find_program(DOT_EXECUTABLE dot)
    if(NOT DOT_EXECUTABLE)
        message(FATAL_ERROR "Graphviz 'dot' executable not found. Please install Graphviz to enable diagram generation in the documentation.")
    endif()

    include(FetchContent)
    FetchContent_Declare(
        doxygen-awesome-css
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
        GIT_TAG v2.4.1
    )
    FetchContent_MakeAvailable(doxygen-awesome-css)
    FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

    set(ROOT "${CMAKE_SOURCE_DIR}")

    # -- Project metadata ------------------------------------------------------
    set(DOXYGEN_PROJECT_NAME        "${PROJECT_NAME}")
    set(DOXYGEN_PROJECT_BRIEF       "${PROJECT_DESCRIPTION}")
    set(DOXYGEN_PROJECT_NUMBER      "${PROJECT_VERSION}")

    # -- HTML / theme ----------------------------------------------------------
    set(DOXYGEN_HTML_EXTRA_STYLESHEET "${AWESOME_CSS_DIR}/doxygen-awesome.css")
    set(DOXYGEN_HTML_COLORSTYLE       "LIGHT")
    set(DOXYGEN_GENERATE_TREEVIEW     YES)

    # -- Input / file patterns -------------------------------------------------
    set(DOXYGEN_FILE_PATTERNS           *.h *.hh *.hpp *.hxx *.cpp *.cc *.cxx *.md)
    set(DOXYGEN_RECURSIVE               YES)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE  "${ROOT}/README.md")
    set(DOXYGEN_STRIP_FROM_PATH         "${ROOT}")
    set(DOXYGEN_STRIP_FROM_INC_PATH     "${ROOT}")

    # -- Image paths (collect every existing assets dir) -----------------------
    set(IMAGE_PATH_CANDIDATES
        "${ROOT}/assets"
        "${ROOT}/docs/assets"
    )

    set(DOXYGEN_IMAGE_PATH)
    foreach(img IN LISTS IMAGE_PATH_CANDIDATES)
        if(EXISTS "${img}")
            list(APPEND DOXYGEN_IMAGE_PATH "${img}")
        endif()
    endforeach()

    # -- Exclusions -----------------------------------------------------------
    set(DOXYGEN_EXCLUDE_PATTERNS
        */build/*
        */.git/*
        */.*
        */CMakeFiles/*
        */tests/*
    )

    # -- Warnings / extraction -------------------------------------------------
    set(DOXYGEN_EXTRACT_ALL          NO)
    set(DOXYGEN_EXTRACT_PRIVATE      NO)
    set(DOXYGEN_EXTRACT_STATIC       YES)
    set(DOXYGEN_GENERATE_LATEX       NO)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED YES)
    set(DOXYGEN_WARN_IF_DOC_ERROR    YES)
    set(DOXYGEN_WARN_NO_PARAMDOC     YES)
    set(DOXYGEN_WARN_AS_ERROR        NO)
    set(DOXYGEN_QUIET                NO)

    # -- Source browsing / graphs -----------------------------------------------
    set(DOXYGEN_SOURCE_BROWSER        YES)
    set(DOXYGEN_INLINE_SOURCES       NO)
    set(DOXYGEN_HAVE_DOT             YES)
    set(DOXYGEN_CLASS_DIAGRAMS       YES)
    set(DOXYGEN_COLLABORATION_GRAPH  YES)
    set(DOXYGEN_INCLUDE_GRAPH        YES)
    set(DOXYGEN_INCLUDED_BY_GRAPH    YES)
    set(DOXYGEN_CALL_GRAPH           YES)
    set(DOXYGEN_CALLER_GRAPH         YES)
    set(DOXYGEN_DOT_GRAPH_MAX_NODES 300)
    set(DOXYGEN_USE_MATHJAX          YES)

    # -- Gather source files ----------------------------------------------------
    # Public headers
    file(GLOB_RECURSE PUBLIC_HEADERS
        "${ROOT}/headers/**/*.hpp"
    )

    # Implementation sources
    file(GLOB_RECURSE SOURCES
        "${ROOT}/sources/**/*.cpp"
    )

    # Top-level docs
    file(GLOB_RECURSE TOP_DOCS
        "${ROOT}/docs/*.md"
        "${ROOT}/docs/**/*.md"
    )

    # Optional well-known documentation files
    set(OPTIONAL_ROOT_DOCS)
    foreach(_f "${ROOT}/README.md" "${ROOT}/CONTRIBUTING.md")
        if(EXISTS "${_f}")
            list(APPEND OPTIONAL_ROOT_DOCS "${_f}")
        endif()
    endforeach()

    # Build the full file list for Doxygen
    set(DOC_INPUT_FILES
        ${PUBLIC_HEADERS}
        ${SOURCES}
        ${TOP_DOCS}
        ${OPTIONAL_ROOT_DOCS}
    )

    list(REMOVE_DUPLICATES DOC_INPUT_FILES)

    doxygen_add_docs(
        evan_docs
        ${DOC_INPUT_FILES}
        ALL
        USE_STAMP_FILE
        COMMENT "Generating unified API documentation with Doxygen"
    )

    # -- Copy all asset directories into html output ----------------------------
    # Doxygen generates its HTML under ${CMAKE_CURRENT_BINARY_DIR}/html when
    # OUTPUT_DIRECTORY is left empty / default.
    set(HTML_OUT "${CMAKE_CURRENT_BINARY_DIR}/html")

    foreach(img IN LISTS DOXYGEN_IMAGE_PATH)
        if(IS_DIRECTORY "${img}")
            get_filename_component(_asset_name "${img}" NAME)
            # Preserve structure relative to root when copying
            file(RELATIVE_PATH _rel "${ROOT}" "${img}")
            set(_dest "${HTML_OUT}/${_rel}")
            add_custom_command(TARGET evan_docs POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${img}"
                    "${_dest}"
                COMMENT "Copying assets directory ${_rel} for documentation"
            )
        endif()
    endforeach()
endfunction()
