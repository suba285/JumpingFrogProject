cmake_minimum_required(VERSION 3.29)
project(JumpingFrogProject)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-lncurses")

add_executable(JumpingFrogProject main.cpp)
