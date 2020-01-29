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
find_package (Vulkan 1.1.130 REQUIRED)
find_package (GLFW3 3.2 REQUIRED)


# Project includes.
include_directories (${Vulkan_INCLUDE_DIRS})
include_directories (${GLFW3_INCLUDE_DIRS})


# Set strict warnings and treat them as errors
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()


# Main executable.
set(cloud_tracer_sources_main
    src/application.cpp
    src/main.cpp
    src/window.cpp)
set(cloud_tracer_sources_vulkan
    src/vulkan/command_pool.cpp
    src/vulkan/device.cpp
    src/vulkan/debug_messenger.cpp
    src/vulkan/instance.cpp
    src/vulkan/memory.cpp
    src/vulkan/semaphore.cpp
    src/vulkan/swapchain.cpp)
set(cloud_tracer_sources_utils)
set(cloud_tracer_headers_main
    src/application.h
    src/window.h)
set(cloud_tracer_headers_vulkan
    src/vulkan/command_pool.h
    src/vulkan/device.h
    src/vulkan/debug_messenger.h
    src/vulkan/exception.h
    src/vulkan/instance.h
    src/vulkan/memory.h
    src/vulkan/object.h
    src/vulkan/semaphore.h
    src/vulkan/swapchain.h)
set(cloud_tracer_headers_utils
    src/utils/ignore_unused.h)
set(cloud_tracer_sources_all
    ${cloud_tracer_sources_main}
    ${cloud_tracer_sources_vulkan}
    ${cloud_tracer_sources_utils})
set(cloud_tracer_headers_all
    ${cloud_tracer_headers_main}
    ${cloud_tracer_headers_vulkan}
    ${cloud_tracer_headers_utils})
source_group("" FILES ${cloud_tracer_sources_main} ${cloud_tracer_headers_main})
source_group("vulkan" FILES ${cloud_tracer_sources_vulkan} ${cloud_tracer_headers_vulkan})
source_group("utils" FILES ${cloud_tracer_sources_utils} ${cloud_tracer_headers_utils})
add_executable (cloud-tracer ${cloud_tracer_sources_all} ${cloud_tracer_headers_all})
target_include_directories(cloud-tracer PRIVATE src)
target_link_libraries(cloud-tracer ${Vulkan_LIBRARIES})
target_link_libraries(cloud-tracer ${GLFW3_LIBRARIES})
