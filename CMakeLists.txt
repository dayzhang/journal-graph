cmake_minimum_required(VERSION 3.0)

project(myproject)

# Set up your source files and add an executable target

set(CMAKE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# Add the source directory to the include path
include_directories(${CMAKE_SOURCE_DIR})

add_executable(main ${CMAKE_SOURCE_DIR}/src/main.cpp ${CMAKE_SOURCE_DIR}/dependencies/simdjson.cpp)

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/main
)

find_package(Catch2 REQUIRED)
include(Catch)