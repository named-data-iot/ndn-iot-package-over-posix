# Root CMake file for NDN-Lite over POSIX
cmake_minimum_required (VERSION 3.13)
project(ndn-lite)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -O0 -ggdb")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O3")
add_library(ndn-lite STATIC)
include(CMakeFiles/ndnlite.cmake)
