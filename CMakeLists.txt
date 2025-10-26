cmake_minimum_required(VERSION 3.10)
project(GridIndex VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Header-only library interface
add_library(grid_index INTERFACE)
target_include_directories(grid_index INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Option to build examples
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Option to build tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(FILES include/grid_index.h
        DESTINATION include)

install(TARGETS grid_index
        EXPORT GridIndexTargets)

install(EXPORT GridIndexTargets
        FILE GridIndexTargets.cmake
        DESTINATION lib/cmake/GridIndex)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "GridIndexConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/GridIndexConfigVersion.cmake"
    DESTINATION lib/cmake/GridIndex
)
