cmake_minimum_required(VERSION 3.10)
project(GridIndexTests)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Test executable
add_executable(test_grid_index test_grid_index.cpp)

# Enable testing
enable_testing()
add_test(NAME grid_index_tests COMMAND test_grid_index)
