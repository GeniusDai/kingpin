cmake_minimum_required(VERSION 2.8.12)
project(KINGPIN CXX)

set(CMAKE_CXX_FLAGS "-g -O0 -Wall -std=c++11")

find_package(GTest REQUIRED)

include_directories("./include")
link_libraries("gtest")
link_libraries("pthread")

add_subdirectory("./lib")
add_subdirectory("./examples")
add_subdirectory("./unittest")

install (
    TARGETS kingpin_static kingpin_shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install (
    DIRECTORY include/kingpin DESTINATION include
)
