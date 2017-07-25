# Compilation flags

find_program (PEAK_COMPILER "mipsel-unknown-gappeak-gcc"
	PATHS "/usr/local/bin" "/usr/local/mipsel-unknown-gappeak/bin/"
		"${MANGO_ROOT}/usr/local/mipsel-unknown-gappeak/bin/")

if (${PEAK_COMPILER} STREQUAL "PEAK_COMPILER-NOTFOUND") 
	message(FATAL_ERROR "I'm not able to find the PEAK compiler: PATH="
		${MANGO_ROOT}/usr/local/mipsel-unknown-gappeak/bin/)
endif (${PEAK_COMPILER} STREQUAL "PEAK_COMPILER-NOTFOUND") 

set(CMAKE_C_COMPILER ${PEAK_COMPILER})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msoft-float -DTARGET_V485T -DDEBUG -DVERBOSE_DBG -std=c99 -Wall")

# Set DEBUG compilation
set(CMAKE_BUILD_TYPE Debug)

# List of source files of the HN-side support
set(HN_SOURCES
	debug.c
	peak/mango_hn.c
	peak/mango_hn_peak.c
)

add_library (mango-devStatic STATIC ${HN_SOURCES})

target_include_directories (mango-devStatic PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories (mango-devStatic PUBLIC ${PROJECT_SOURCE_DIR}/dev/include)

install (TARGETS mango-devStatic DESTINATION ${MANGO_ROOT}/lib)
set_target_properties(mango-devStatic PROPERTIES OUTPUT_NAME mango-dev)


