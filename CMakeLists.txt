cmake_minimum_required(VERSION 3.7)


# Set compiler version (currently C++14).
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Setup project.
project(cloud-tracer)


# Modules.
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}/cmake/modules
)


# Find required packages.
find_package(Vulkan 1.1.130 REQUIRED)
find_package(GLFW3 3.2 REQUIRED)


# Set strict warnings and treat them as errors
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()


# Sources.
set(CLOUD_TRACER_SOURCES_MAIN
    src/application.cpp
    src/main.cpp
    src/window.cpp
)
set(CLOUD_TRACER_SOURCES_VULKAN
    src/vulkan/command_pool.cpp
    src/vulkan/device.cpp
    src/vulkan/debug_messenger.cpp
    src/vulkan/instance.cpp
    src/vulkan/memory.cpp
    src/vulkan/swapchain.cpp
    src/vulkan/synchronization.cpp
)
set(CLOUD_TRACER_SOURCES_UTILS
)
set(CLOUD_TRACER_HEADERS_MAIN
    src/application.h
    src/window.h
)
set(CLOUD_TRACER_HEADERS_VULKAN
    src/vulkan/command_pool.h
    src/vulkan/device.h
    src/vulkan/debug_messenger.h
    src/vulkan/exception.h
    src/vulkan/instance.h
    src/vulkan/memory.h
    src/vulkan/object.h
    src/vulkan/swapchain.h
    src/vulkan/synchronization.h
)
set(CLOUD_TRACER_HEADERS_UTILS
    src/utils/ignore_unused.h
)
set(CLOUD_TRACER_SOURCES_ALL
    ${CLOUD_TRACER_SOURCES_MAIN}
    ${CLOUD_TRACER_SOURCES_VULKAN}
    ${CLOUD_TRACER_SOURCES_UTILS}
)
set(CLOUD_TRACER_HEADERS_ALL
    ${CLOUD_TRACER_HEADERS_MAIN}
    ${CLOUD_TRACER_HEADERS_VULKAN}
    ${CLOUD_TRACER_HEADERS_UTILS}
)


# Group sources for IDE.
source_group("" FILES ${CLOUD_TRACER_SOURCES_MAIN} ${CLOUD_TRACER_HEADERS_MAIN})
source_group("vulkan" FILES ${CLOUD_TRACER_SOURCES_VULKAN} ${CLOUD_TRACER_HEADERS_VULKAN})
source_group("utils" FILES ${CLOUD_TRACER_SOURCES_UTILS} ${CLOUD_TRACER_HEADERS_UTILS})


# Main executable.
add_executable (cloud-tracer ${CLOUD_TRACER_SOURCES_ALL} ${CLOUD_TRACER_HEADERS_ALL})
target_include_directories(cloud-tracer
    PRIVATE
    src
)


# Dependencies
target_link_libraries(cloud-tracer Vulkan::Vulkan)
target_link_libraries(cloud-tracer glfw)
