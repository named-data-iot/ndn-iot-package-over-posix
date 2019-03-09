# Root CMake file for NDN-Lite over POSIX
cmake_minimum_required (VERSION 3.11)
project(ndn-lite)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -O0 -ggdb")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(DIR_CMAKEFILES "${PROJECT_SOURCE_DIR}/CMakeInputs")
set(DIR_NDN_LITE "${PROJECT_SOURCE_DIR}/ndn-lite")
set(DIR_ADAPTATION "${PROJECT_SOURCE_DIR}/adaptation")
include_directories(${PROJECT_SOURCE_DIR})

# NDN-Lite static library
add_library(ndn-lite STATIC)
include(${DIR_CMAKEFILES}/ndnlite.cmake)

# Adaptation
include(${DIR_CMAKEFILES}/adaptation.cmake)

# Unit test program
add_executable(unittest)
target_link_libraries(unittest ndn-lite)
include(${DIR_CMAKEFILES}/unittest.cmake)

# Doxygen
# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOCS "Build documentation" OFF)
find_package(Doxygen
  OPTIONAL_COMPONENTS dot
)
if (DOXYGEN_FOUND AND BUILD_DOCS)
  set(DOXYGEN_PROJECT_LOGO ${PROJECT_SOURCE_DIR}/docs/logo.jpeg)
  set(DOXYGEN_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/docs)
  set(DOXYGEN_TAB_SIZE 2)
  set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
  set(DOXYGEN_EXTRACT_ALL YES)
  set(DOXYGEN_INCLUDE_GRAPH NO)
  set(DOXYGEN_INCLUDED_BY_GRAPH NO)
  set(DOXYGEN_UML_LOOK YES)
  set(DOXYGEN_UML_LIMIT_NUM_FIELDS 3)
  set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
  doxygen_add_docs(docs
    ${PROJECT_SOURCE_DIR}/docs
    ${PROJECT_SOURCE_DIR}/ndn-lite
    COMMENT "Generating API documentation with Doxygen"
  )
else ()
  message("Documentation skipped")
endif ()
