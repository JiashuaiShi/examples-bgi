cmake_minimum_required(VERSION 3.18)
project(samCompare)

set(CMAKE_CXX_STANDARD 11)

add_executable(samCompare main.cpp lib.hpp queue.hpp)
