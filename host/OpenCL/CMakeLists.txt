# Basic makelist
cmake_minimum_required(VERSION 3.10)
project(OpenCL VERSION 1.0.1)

add_subdirectory(src)
# add_subdirectory(test)

# # Basic library generation
# cmake_minimum_required (VERSION 3.0)
# project (OpenCL VERSION 1.0.1)

# add_library(opencl SHARED
#   src/cl.cpp
# )

# set_target_properties(opencl PROPERTIES VERSION ${PROJECT_VERSION})
# set_target_properties(opencl PROPERTIES PUBLIC_HEADER include/opencl.h)

# target_include_directories(opencl PRIVATE include)
# target_include_directories(opencl PRIVATE src)

# # Installation

# include(GNUInstallDirs)

# install(TARGETS opencl
#   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#   PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )

# configure_file(opencl.pc.in opencl.pc @ONLY)

# install(FILES ${CMAKE_BINARY_DIR}/opencl.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)