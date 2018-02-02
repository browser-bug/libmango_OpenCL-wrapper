if (CONFIG_LIBMANGO_NUPLUS)


# Compilation flags

find_program (NUPLUS_COMPILER "clang"
	PATHS "/usr/local/bin" "/usr/local/llvm-nuplus/bin/"
		"${MANGO_ROOT}/usr/local/llvm-nuplus/bin/")

if (${NUPLUS_COMPILER} STREQUAL "NUPLUS_COMPILER-NOTFOUND") 
	message(FATAL_ERROR "I'm not able to find the NUPLUS compiler: PATH="
		${MANGO_ROOT}/usr/local/llvm-nuplus/bin/)
endif (${NUPLUS_COMPILER} STREQUAL "NUPLUS_COMPILER-NOTFOUND") 

set(CMAKE_C_COMPILER ${NUPLUS_COMPILER})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --target=nuplus -Wall -W")

# Set DEBUG compilation
set(CMAKE_BUILD_TYPE Debug)

# List of source files of the HN-side support
set(HN_SOURCES
	mango_hn.c
	mango_hn_nuplus.c
)

add_library (mango-dev-nuplusStatic STATIC ${HN_SOURCES})

target_include_directories (mango-dev-nuplusStatic PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories (mango-dev-nuplusStatic PUBLIC ${PROJECT_SOURCE_DIR}/dev/include)
target_include_directories (mango-dev-nuplusStatic PUBLIC ${PROJECT_SOURCE_DIR}/dev/include/nuplus)

install (TARGETS mango-dev-nuplusStatic DESTINATION ${MANGO_ROOT}/lib)
set_target_properties(mango-dev-nuplusStatic PROPERTIES OUTPUT_NAME mango-dev-nuplus)


endif (CONFIG_LIBMANGO_NUPLUS)
