cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

project(imgui VERSION 1.0)

file(GLOB _SOURCES  "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
                    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB EXTERNAL_H  "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_subdirectory(3rdlibs)

add_library(${PROJECT_NAME} SHARED ${_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        PUBLIC_HEADER "${EXTERNAL_H}"
                        OUTPUT_NAME "${PROJECT_NAME}")

target_include_directories(${PROJECT_NAME} PUBLIC
                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                        $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

include(${PROJECT_SOURCE_DIR}/cmake/InstallLib.cmake)
install_lib_with_module(LIB_NAME ${PROJECT_NAME} MODULE_NAME ${PROJECT_NAME} CMAKE_PATH ${PROJECT_SOURCE_DIR}/cmake)