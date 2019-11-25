cmake_minimum_required(VERSION 3.5)

# Compilation flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall --pedantic -std=c99")

if(CONFIG_LIBMANGO_GN)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGNEMU")
endif(CONFIG_LIBMANGO_GN)

# Set DEBUG compilation
set(CMAKE_BUILD_TYPE Debug)

if (NOT CONFIG_SAMPLES_MATRIX_MULTIPLICATION)
	return()
endif(NOT CONFIG_SAMPLES_MATRIX_MULTIPLICATION)

set(MANGO_ROOT "$ENV{MANGO_ROOT}" CACHE STRING "MANGO installation root directory")

set(MATRIX_MULTIPLICATION_SOURCES matrix_multiplication_host.c)


add_executable (matrix_multiplication_test ${MATRIX_MULTIPLICATION_SOURCES})


target_include_directories(matrix_multiplication_test PUBLIC ${MANGO_ROOT}/include/libmango/)

if (NOT BOSP_PATH) 
	message(FATAL_ERROR "You have to specify -DBOSP_PATH=\"/path/to/bbque\"!")
endif (NOT BOSP_PATH)

target_link_libraries(matrix_multiplication_test PUBLIC ${BOSP_PATH}/lib/bbque/libbbque_pms.so)
target_link_libraries(matrix_multiplication_test PUBLIC ${BOSP_PATH}/lib/bbque/libbbque_tg.so)

find_library(LIBMANGO mango
	HINTS ${MANGO_ROOT}/lib)

target_link_libraries(matrix_multiplication_test PUBLIC ${LIBMANGO})

install(TARGETS matrix_multiplication_test DESTINATION ${MANGO_ROOT}/usr/bin)

add_subdirectory(kernel)

