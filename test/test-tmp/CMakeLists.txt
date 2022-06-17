cmake_minimum_required(VERSION 3.18)
project(sam_depth)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -DNDEBUG  -fopenmp")   #

add_executable(test main.cpp)
