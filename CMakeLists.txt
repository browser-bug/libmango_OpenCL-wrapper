cmake_minimum_required (VERSION 2.8.11)
project (LIBMANGO)

set(MANGO_ROOT "/" CACHE STRING "MANGO installation root directory")

include(../global.cmake)

# Set DEBUG compilation
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wextra -pedantic -DDEBUG")
if (BUILD_TYPE STREQUAL "Debug")
  set (CMAKE_BUILD_TYPE Debug)
  set (DEBUG ON)
else (BUILD_TYPE STREQUAL "Debug")
  set (CMAKE_BUILD_TYPE Release)
  set (CMAKE_CXX_FLAGS "-O3")
endif (BUILD_TYPE STREQUAL "Debug")
message (STATUS "Build type: " ${CMAKE_BUILD_TYPE})


add_subdirectory (include)
add_subdirectory (host)
add_subdirectory (dev)
add_subdirectory (docs)

