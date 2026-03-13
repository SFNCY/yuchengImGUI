cmake_minimum_required(VERSION 3.30.0)
project(demoGlfwOpenGL C CXX)

set(CMAKE_CXX_STANDARD 17)

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

file(GLOB sources ./*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} ${_LINKER_LIBS} imgui::imgui)
