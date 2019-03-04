# Root CMake file for NDN-Lite over POSIX
cmake_minimum_required (VERSION 3.11)
project(ndn-lite)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -O0 -ggdb")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(DIR_CMAKEFILES "${PROJECT_SOURCE_DIR}/CMakeInputs")
set(DIR_NDN_LITE "${PROJECT_SOURCE_DIR}/ndn-lite")
include_directories(${PROJECT_SOURCE_DIR})

# NDN-Lite static library
add_library(ndn-lite STATIC)
include(${DIR_CMAKEFILES}/ndnlite.cmake)

# Unit test program
add_executable(unittest)
target_link_libraries(unittest ndn-lite)
include(${DIR_CMAKEFILES}/unittest.cmake)

# Doxygen
# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOCS "Build documentation" OFF)
find_package(Doxygen)
if (DOXYGEN_FOUND AND BUILD_DOCS)
    # set input and output files
    set(DOXYGEN_IN ${PROJECT_SOURCE_DIR}/docs/Doxyfile)
    set(DOXYGEN_OUT ${PROJECT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( docs ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else ()
  message("Documentation skipped")
endif ()
