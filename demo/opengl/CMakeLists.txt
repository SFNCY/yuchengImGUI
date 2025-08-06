cmake_minimum_required(VERSION 3.30.0)
project(demoGlfwOpenGL C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
list(APPEND _LINKER_LIBS glfw X11 pthread GL dl vulkan Xxf86vm Xrandr Xi)

find_package(imgui)

file(GLOB sources ./*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} ${_LINKER_LIBS} imgui::imgui)
