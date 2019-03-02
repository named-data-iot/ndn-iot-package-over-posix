# Root CMake file for NDN-Lite over POSIX
cmake_minimum_required (VERSION 3.11)
project(ndn-lite)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -O0 -ggdb")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(DIR_CMAKEFILES "${PROJECT_SOURCE_DIR}/CMakeInputs")
set(DIR_NDN_LITE "${PROJECT_SOURCE_DIR}/ndn-lite")
include_directories(${PROJECT_SOURCE_DIR})

add_library(ndn-lite STATIC)
include(${DIR_CMAKEFILES}/ndnlite.cmake)

add_executable(unittest)
target_link_libraries(unittest ndn-lite)
include(${DIR_CMAKEFILES}/unittest.cmake)
