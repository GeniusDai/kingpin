cmake_minimum_required(VERSION 2.6)
project(KINGPIN CXX)

set(CMAKE_CXX_FLAGS "-g -O0 -Wall -std=c++11")

include_directories("./include")
link_libraries("gtest")
link_libraries("pthread")

add_subdirectory("./lib")
add_subdirectory("./examples")
add_subdirectory("./unittest")
