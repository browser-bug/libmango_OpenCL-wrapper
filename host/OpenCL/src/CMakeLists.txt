# List of source files of OpenCL library
file(GLOB OPENCL_SOURCES *.cpp)
            
# Build for convenience both static and dynamic library
add_library (opencl SHARED ${OPENCL_SOURCES})
add_library (openclStatic STATIC ${OPENCL_SOURCES})

target_include_directories (opencl PUBLIC ${OpenCL_SOURCE_DIR}/include)
target_include_directories(opencl PUBLIC ${OpenCL_SOURCE_DIR}/../include)

#target_link_libraries(opencl mango)
#target_link_libraries(opencl mangoStatic)


