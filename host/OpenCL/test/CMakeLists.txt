include_directories(${OpenCL_SOURCE_DIR}/include)
link_directories(${OpenCL_SOURCE_DIR}/src)
add_executable(Test matrix_multiplication.c)
target_link_libraries(Test opencl)