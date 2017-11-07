cmake_minimum_required (VERSION 2.8.11)
project (LIBMANGO)

set(MANGO_ROOT "/" CACHE STRING "MANGO installation root directory")

include(../global.cmake)

add_subdirectory (include)
add_subdirectory (host)
add_subdirectory (dev)
add_subdirectory (docs)

