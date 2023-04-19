cmake_minimum_required(VERSION 3.0)

project(myproject)

# Set up your source files and add an executable target
set(SOURCES main.cpp)
add_executable(main ${SOURCES})

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/main
)

find_package(Catch2 REQUIRED)
include(Catch)