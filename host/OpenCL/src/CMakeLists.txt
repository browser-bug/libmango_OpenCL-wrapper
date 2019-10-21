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

# BOSP libraries inclusion
target_include_directories (opencl PUBLIC ${BOSP_PATH}/include)
target_include_directories (openclStatic PUBLIC ${BOSP_PATH}/include)
target_include_directories (opencl PUBLIC ${BOSP_PATH}/include/bbque)
target_include_directories (openclStatic PUBLIC ${BOSP_PATH}/include/bbque)

target_include_directories (opencl PUBLIC ${MANGO_ROOT}/include)
target_include_directories (openclStatic PUBLIC ${MANGO_ROOT}/include)

# Probably not needed but added for completition

# #link_directories(${BOSP_PATH}/lib/bbque)
# target_link_libraries(opencl PUBLIC ${BOSP_PATH}/lib/bbque/libbbque_pms.so)
# target_link_libraries(openclStatic PUBLIC ${BOSP_PATH}/lib/bbque/libbbque_pms.so)
# target_link_libraries(opencl PUBLIC ${BOSP_PATH}/lib/bbque/libbbque_tg.so)
# target_link_libraries(openclStatic PUBLIC ${BOSP_PATH}/lib/bbque/libbbque_tg.so)

# # HN Library linking
# if (CONFIG_LIBMANGO_GN)
# 	target_link_libraries(opencl PUBLIC ${MANGO_ROOT}/lib/libgn.so)
# 	target_link_libraries(openclStatic PUBLIC ${MANGO_ROOT}/lib/libgn.a)
# else (CONFIG_LIBMANGO_GN)
# 	target_link_libraries(opencl PUBLIC ${MANGO_ROOT}/lib/libhn.so)
# 	target_link_libraries(openclStatic PUBLIC ${MANGO_ROOT}/lib/libhn.a)
# endif (CONFIG_LIBMANGO_GN)

target_link_libraries(opencl PUBLIC ${MANGO_ROOT}/lib/libmango.so)
target_link_libraries(openclStatic PUBLIC ${MANGO_ROOT}/lib/libmango.a)


