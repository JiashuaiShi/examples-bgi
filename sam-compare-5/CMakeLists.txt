cmake_minimum_required(VERSION 3.18)
project(samCompare)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -DNDEBUG  -fopenmp" )   # release包优化

add_executable(samCompare main.cpp safeHashMap.hpp)
