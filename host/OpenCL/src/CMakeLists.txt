# List of source files of OpenCL library
set(OPENCL_SOURCES	
            cl.cpp
			)
            
# Build for convenience both static and dynamic library
add_library (opencl SHARED ${OPENCL_SOURCES})
add_library (openclStatic STATIC ${OPENCL_SOURCES})

target_include_directories (opencl PUBLIC ${OpenCL_SOURCE_DIR}/include)
target_include_directories (openclStatic PUBLIC ${OpenCL_SOURCE_DIR}/include)
target_include_directories (opencl PUBLIC ${OpenCL_SOURCE_DIR}/../include)
target_include_directories (openclStatic PUBLIC ${OpenCL_SOURCE_DIR}/../include)

target_link_libraries(opencl PUBLIC ${MANGO_ROOT}/lib/libmango.so)
target_link_libraries(openclStatic PUBLIC ${MANGO_ROOT}/lib/libmango.a)


