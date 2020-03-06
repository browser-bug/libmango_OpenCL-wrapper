include_directories(${OpenCL_SOURCE_DIR}/include)
link_directories(${OpenCL_SOURCE_DIR}/src)

add_executable(Test1 matrix_multiplication.c)
add_executable(Test2 vector_addition.c)

target_link_libraries(Test1 opencl)
target_link_libraries(Test2 opencl) # this need the libm (math.h)

target_link_libraries(Test2 m)