cmake_minimum_required(VERSION 3.1)

project(HelloWorld)

# Name the executable and link the source files to build it
add_executable("${PROJECT_NAME}" "main.c")

# Installation specifing targets and destination for them
install(TARGETS "${POJECT_NAME}" DESTINATION bin)       # ${CMAKE_INSTALL_PREFIX}/bin       so bin is a relative path
install(FILES "main.c" DESTINATION src)                 # ${CMAKE_INSTALL_PREFIX}/src