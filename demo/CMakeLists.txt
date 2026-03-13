cmake_minimum_required(VERSION 3.30.0)

add_subdirectory(opengl)
# Vulkan 只在非 macOS 平台编译
if(NOT APPLE)
    add_subdirectory(vulkan)
endif()