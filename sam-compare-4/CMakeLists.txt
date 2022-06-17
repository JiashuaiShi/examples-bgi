cmake_minimum_required(VERSION 3.18)
project(samCompare)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-O3 -DNDEBUG " )   # release包优化

add_executable(samCompare main.cpp)
