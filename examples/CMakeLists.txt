cmake_minimum_required(VERSION 3.10)
project(GridIndexExamples)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Basic usage example
add_executable(basic_usage basic_usage.cpp)

# Performance example
add_executable(performance performance_example.cpp)
