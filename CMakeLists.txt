cmake_minimum_required (VERSION 2.8.11)
project (LIBMANGO)

set(MANGO_ROOT "/opt/mango" CACHE STRING "MANGO installation root directory")

include(~/mangolibs/global.cmake)
include(~/mangolibs/bosp/build/configuration/bosp-config.cmake)

# Set DEBUG compilation
if (CONFIG_BBQUE_BUILD_DEBUG)
  set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wextra -pedantic -DDEBUG -DBBQUE_DEBUG")
else (CONFIG_BBQUE_BUILD_DEBUG)
  set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wextra -pedantic -DDEBUG")
endif (CONFIG_BBQUE_BUILD_DEBUG)

if (BUILD_TYPE STREQUAL "Debug")
  set (CMAKE_BUILD_TYPE Debug)
  set (DEBUG ON)
else (BUILD_TYPE STREQUAL "Debug")
  set (CMAKE_BUILD_TYPE Release)
  set (CMAKE_CXX_FLAGS "-O3")
endif (BUILD_TYPE STREQUAL "Debug")
message (STATUS "Build type: " ${CMAKE_BUILD_TYPE})

if (CONFIG_LIBMANGO_PROFILING_MODE)
  set (NR_CORES_PER_PROC ${CONFIG_LIBMANGO_NR_CORES_MAX})
  message (STATUS "Cores per unit: " ${NR_CORES_PER_PROC})
endif (CONFIG_LIBMANGO_PROFILING_MODE)

configure_file (
	"${PROJECT_SOURCE_DIR}/libmango.conf.in"
	"${PROJECT_BINARY_DIR}/libmango.conf"
)

configure_file(
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/include/libmango/config.h"
)


include_directories(
	"${PROJECT_BINARY_DIR}/include/libmango/"
)

add_subdirectory (include)
add_subdirectory (host)
add_subdirectory (dev)
add_subdirectory (docs)

install (FILES "${PROJECT_BINARY_DIR}/libmango.conf"
	DESTINATION "${BOSP_PATH}/etc/bbque/"
)

install (FILES "${PROJECT_BINARY_DIR}/include/libmango/config.h"
	DESTINATION "${MANGO_ROOT}/include/libmango/"
)
