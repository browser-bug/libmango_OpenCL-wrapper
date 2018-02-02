if (CONFIG_LIBMANGO_GN)


# Compilation flags

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -DDEBUG -DGNEMU")

# Set DEBUG compilation
set(CMAKE_BUILD_TYPE Debug)

# List of source files of the HN-side support
set(GN_SOURCES
	debug.c
	mango_hn.c
	mango_hn_gn.c
)

add_library (mango-dev-gnStatic STATIC ${GN_SOURCES})

target_include_directories (mango-dev-gnStatic PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories (mango-dev-gnStatic PUBLIC ${PROJECT_SOURCE_DIR}/dev/include)
target_include_directories (mango-dev-gnStatic PUBLIC ${PROJECT_SOURCE_DIR}/dev/include/gn)

install (TARGETS mango-dev-gnStatic DESTINATION ${MANGO_ROOT}/lib)
set_target_properties(mango-dev-gnStatic PROPERTIES OUTPUT_NAME mango-dev-gn)


endif (CONFIG_LIBMANGO_GN)
