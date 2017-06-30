cmake_minimum_required (VERSION 2.8.11)
project (LIBMANGO)

set(MANGO_ROOT "/" CACHE STRING "MANGO installation root directory")

add_subdirectory (include)
add_subdirectory (host)
add_subdirectory (dev)

