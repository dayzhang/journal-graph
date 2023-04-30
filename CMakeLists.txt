cmake_minimum_required(VERSION 3.0)

project(myproject)

# Set up your source files and add an executable target

set(CMAKE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR})

add_executable(main ${CMAKE_SOURCE_DIR}/src/main.cpp)

add_executable(run_tests ${CMAKE_SOURCE_DIR}/catch_tests/tests.cpp)

target_link_libraries(run_tests Catch2::Catch2WithMain)

include(CTest)
find_package(Catch2 REQUIRED)
include(Catch)
include_directories(${Catch2_INCLUDE_DIRS})

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/main ${CMAKE_BINARY_DIR}/run_tests
)
