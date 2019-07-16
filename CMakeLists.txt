cmake_minimum_required(VERSION 3.7)


# Set compiler version (currently C++14).
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Setup project.
project(cloud-tracer)

set (CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}/cmake/modules
)


# Find required packages.
find_package (Vulkan REQUIRED)
find_package (GLFW3 3.2 REQUIRED)


# Project includes.
include_directories (${Vulkan_INCLUDE_DIRS})
include_directories (${GLFW3_INCLUDE_DIRS})


# Main executable.
add_executable (cloud-tracer src/main.cpp)
target_link_libraries(cloud-tracer ${Vulkan_LIBRARIES})
target_link_libraries(cloud-tracer ${GLFW3_LIBRARIES})

