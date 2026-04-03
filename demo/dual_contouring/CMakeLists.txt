cmake_minimum_required(VERSION 3.30.0)
project(demoDualContouring C CXX)

set(CMAKE_CXX_STANDARD 17)

# 设置 UTF-8 编码支持
if(WIN32)
    add_compile_options(/utf-8)
endif()

# 查找 GLFW
find_package(glfw3 REQUIRED)

# 根据平台设置不同的链接库
if(WIN32)
    list(APPEND _LINKER_LIBS glfw opengl32 gdi32 user32 kernel32)
elseif(APPLE)
    list(APPEND _LINKER_LIBS glfw "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
else()
    # Linux
    list(APPEND _LINKER_LIBS glfw X11 pthread GL dl Xxf86vm Xrandr Xi)
endif()

find_package(imgui)

# 编译所有源文件（包括 DC 组件）
file(GLOB sources ./*.cpp)
list(FILTER sources EXCLUDE REGEX ".*test_.*")

# 添加 backends 源文件
list(APPEND sources 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../backends/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../backends/imgui_impl_opengl3.cpp
)

add_executable(${PROJECT_NAME} ${sources})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../backends)
target_link_libraries(${PROJECT_NAME} ${_LINKER_LIBS} imgui::imgui)