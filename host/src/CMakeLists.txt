# Compilation flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall --pedantic -std=c++14 -Wno-deprecated-declarations")

# Set DEBUG compilation
set(CMAKE_BUILD_TYPE Debug)

# List of source files of HN library
set(LIBHN_SOURCES	buffer.cpp
			context.cpp
			event.cpp
			kernel.cpp
			kernel_arguments.cpp
			logger.cpp
			mango.cpp
			mm.cpp
			task_graph.cpp
			)


# Build for convenience both static and dynamic library
add_library (mango SHARED ${LIBHN_SOURCES})
add_library (mangoStatic STATIC ${LIBHN_SOURCES})

set_target_properties(mango PROPERTIES COMPILE_FLAGS "-fPIC")

target_include_directories (mango PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories (mangoStatic PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories (mango PUBLIC ${PROJECT_SOURCE_DIR}/host/include)
target_include_directories (mangoStatic PUBLIC ${PROJECT_SOURCE_DIR}/host/include)

if (NOT BOSP_PATH) 
	message(FATAL_ERROR "You have to specify -DBOSP_PATH=\"/path/to/bbque\"!")
endif (NOT BOSP_PATH)

target_include_directories (mango PUBLIC ${BOSP_PATH}/out/include)
target_include_directories (mangoStatic PUBLIC ${BOSP_PATH}/out/include)
target_include_directories (mango PUBLIC ${BOSP_PATH}/barbeque/include)
target_include_directories (mangoStatic PUBLIC ${BOSP_PATH}/barbeque/include)

#link_directories(${BOSP_PATH}/out/lib/bbque)
target_link_libraries(mango PUBLIC ${BOSP_PATH}/out/lib/bbque/libbbque_pms.so)
target_link_libraries(mangoStatic PUBLIC ${BOSP_PATH}/out/lib/bbque/libbbque_pms.so)
target_link_libraries(mango PUBLIC ${BOSP_PATH}/out/lib/bbque/libbbque_tg.so)
target_link_libraries(mangoStatic PUBLIC ${BOSP_PATH}/out/lib/bbque/libbbque_tg.so)

# HN Library linking
link_directories(/usr/local/lib)
target_link_libraries(mango PUBLIC hn)
target_link_libraries(mangoStatic PUBLIC hn)


install (TARGETS mango DESTINATION lib)
install (TARGETS mangoStatic DESTINATION lib)
set_target_properties(mangoStatic PROPERTIES OUTPUT_NAME mango)

