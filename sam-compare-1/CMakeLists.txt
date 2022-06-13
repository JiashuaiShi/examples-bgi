cmake_minimum_required(VERSION 3.18)
project(samCompare)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(samCompare main.cpp compare.hpp safeQueue.hpp buildMap.hpp global.hpp MyQueue.hpp)
