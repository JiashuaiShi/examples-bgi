cmake_minimum_required(VERSION 3.18)
project(samCompare)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -DNDEBUG " )   # release包优化
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(samCompare main.cpp safeQueue.hpp)
