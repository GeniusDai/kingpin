cmake_minimum_required(VERSION 2.6)
project(examples CXX)

set(CXX_STANDARD C++11)

set(
    CMAKE_CXX_FLAGS
    "-g -O0 -std=c++11 -pthread -Wall"
)

include_directories("./include")

add_subdirectory("./lib")
add_subdirectory("./examples")
