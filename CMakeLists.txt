cmake_minimum_required(VERSION 3.24)
project(virtual_reality)

enable_testing()

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(dependencies)
add_subdirectory(src)