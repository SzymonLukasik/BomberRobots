cmake_minimum_required(VERSION 3.1)

project(ROBOTS-CLIENT)

set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Wconversion -Werror -O2 -std=gnu++20 -pthread")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(src)

