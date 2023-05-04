cmake_minimum_required(VERSION 3.0)

project(myproject)

# Set up your source files and add an executable target

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -g -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR})

add_executable(main ${CMAKE_SOURCE_DIR}/src/main.cpp ${CMAKE_SOURCE_DIR}/lib/simdjson.cpp ${CMAKE_SOURCE_DIR}/graph/authorGraph.cpp ${CMAKE_SOURCE_DIR}/graph/journalGraph.cpp)
add_executable(parse ${CMAKE_SOURCE_DIR}/src/parse.cpp ${CMAKE_SOURCE_DIR}/lib/simdjson.cpp ${CMAKE_SOURCE_DIR}/parsing/parsing.cpp ${CMAKE_SOURCE_DIR}/graph/journalGraph.cpp
${CMAKE_SOURCE_DIR}/graph/authorGraph.cpp)
add_executable(db_interface ${CMAKE_SOURCE_DIR}/src/db_interface.cpp ${CMAKE_SOURCE_DIR}/graph/journalGraph.cpp ${CMAKE_SOURCE_DIR}/graph/authorGraph.cpp)
add_executable(paper_game ${CMAKE_SOURCE_DIR}/src/paper_game.cpp ${CMAKE_SOURCE_DIR}/graph/journalGraph.cpp)

add_executable(run_tests ${CMAKE_SOURCE_DIR}/catch_tests/tests.cpp ${CMAKE_SOURCE_DIR}/graph/authorGraph.cpp ${CMAKE_SOURCE_DIR}/graph/journalGraph.cpp)

target_link_libraries(run_tests Catch2::Catch2WithMain)

include(CTest)
find_package(Catch2 REQUIRED)
include(Catch)
include_directories(${Catch2_INCLUDE_DIRS})

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/main ${CMAKE_BINARY_DIR}/run_tests
)
