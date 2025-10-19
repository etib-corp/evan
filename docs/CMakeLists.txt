cmake_minimum_required(VERSION 3.10)

# Find Doxygen package (keep usage compatible across CMake versions and CI images)
find_package(Doxygen REQUIRED)

if(NOT Doxygen_FOUND)
    message(FATAL_ERROR "Doxygen not found. Documentation will not be built. Install doxygen and try again.")
endif()

# Detect Graphviz 'dot' executable (not all FindDoxygen modules expose components
# uniformly across CMake versions/runners). If present, enable diagrams.
find_program(DOT_EXECUTABLE dot)
if(DOT_EXECUTABLE)
    set(DOXYGEN_HAVE_DOT YES)
else()
    set(DOXYGEN_HAVE_DOT NO)
endif()

# Set up doxygen-awesome-css theme (optional styling)
include(FetchContent)
FetchContent_Declare(
    doxygen-awesome-css
    URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(doxygen-awesome-css)
FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

# Configure Doxygen settings using modern CMake variables
# These variables are automatically processed by doxygen_add_docs()
set(DOXYGEN_PROJECT_BRIEF "Evan is a component-based UI framework library built around a primitive rendering system. It's designed for building user interfaces across XR platforms and traditional platforms with custom rendering backends and input handling. The framework is lightweight, flexible, and easy to integrate into existing projects.")
set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated-docs)

# Configure appearance and theme
set(DOXYGEN_HTML_EXTRA_STYLESHEET "${AWESOME_CSS_DIR}/doxygen-awesome.css")
set(DOXYGEN_HTML_COLORSTYLE "LIGHT")
set(DOXYGEN_GENERATE_TREEVIEW YES)

# Configure input files and patterns
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md")
set(DOXYGEN_FILE_PATTERNS *.cpp *.hpp *.h *.md)
set(DOXYGEN_EXCLUDE_PATTERNS
    */build/*
    */cmake/*
    */.*
    */CMakeFiles/*
)

# Additional options for better documentation quality
set(DOXYGEN_EXTRACT_ALL NO)
set(DOXYGEN_EXTRACT_PRIVATE NO)
set(DOXYGEN_EXTRACT_STATIC YES)
set(DOXYGEN_GENERATE_LATEX NO)
set(DOXYGEN_WARN_IF_UNDOCUMENTED YES)
set(DOXYGEN_WARN_IF_DOC_ERROR YES)
set(DOXYGEN_QUIET YES)

# Source browsing
set(DOXYGEN_SOURCE_BROWSER YES)
set(DOXYGEN_INLINE_SOURCES NO)

# Class diagrams and graphs (requires dot/Graphviz)
if(TARGET Doxygen::dot)
    set(DOXYGEN_HAVE_DOT YES)
    set(DOXYGEN_CLASS_DIAGRAMS YES)
    set(DOXYGEN_COLLABORATION_GRAPH YES)
    set(DOXYGEN_INCLUDE_GRAPH YES)
    set(DOXYGEN_INCLUDED_BY_GRAPH YES)
    set(DOXYGEN_CALL_GRAPH YES)
    set(DOXYGEN_CALLER_GRAPH YES)
    set(DOXYGEN_DOT_MULTI_TARGETS YES)
else()
    set(DOXYGEN_HAVE_DOT NO)
    set(DOXYGEN_CLASS_DIAGRAMS NO)
endif()

# Collect source files for documentation
file(GLOB_RECURSE HEADER_FILES "${CMAKE_SOURCE_DIR}/headers/*.hpp" "${CMAKE_SOURCE_DIR}/headers/*.h")
file(GLOB_RECURSE DOC_FILES "${CMAKE_SOURCE_DIR}/docs/*.md")

# Create documentation using modern doxygen_add_docs function
doxygen_add_docs(
    docs
    ${HEADER_FILES}
    ${CMAKE_SOURCE_DIR}/README.md
    ${DOC_FILES}
    ALL
    USE_STAMP_FILE
    COMMENT "Generating API documentation with Doxygen"
)

# Print documentation configuration summary
message(STATUS "Documentation configuration:")
message(STATUS "  - Doxygen executable: ${DOXYGEN_EXECUTABLE}")
message(STATUS "  - Doxygen version: ${DOXYGEN_VERSION}")
message(STATUS "  - Project name: ${PROJECT_NAME}")
message(STATUS "  - Project version: ${PROJECT_VERSION}")
message(STATUS "  - Output directory: ${DOXYGEN_OUTPUT_DIRECTORY}/html")
message(STATUS "  - Dot support: ${DOXYGEN_HAVE_DOT}")
if(TARGET Doxygen::dot)
    message(STATUS "  - Dot executable: Available")
else()
    message(STATUS "  - Dot executable: Not available")
endif()
