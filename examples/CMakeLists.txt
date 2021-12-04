cmake_minimum_required(VERSION 2.6)
project(examples CXX)

set(
    CMAKE_CXX_FLAGS
    "-g -O0 -std=c++11 -pthread -Wall"
)

include_directories("../include")

add_subdirectory(quick_start)
add_subdirectory(big_file_transfer)
add_subdirectory(chinese_chess_game)
