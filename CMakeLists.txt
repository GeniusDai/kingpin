cmake_minimum_required(VERSION 2.6)
project(examples CXX)

set(CXX_STANDARD C++11)
set(CMAKE_CXX_FLAGS "-g -O0 -Wall")

include_directories("./include")
link_libraries("gtest")
link_libraries("pthread")

add_subdirectory("./lib")
add_subdirectory("./examples")
add_subdirectory("./unittest")
