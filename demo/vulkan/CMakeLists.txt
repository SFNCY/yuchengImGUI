cmake_minimum_required(VERSION 3.30.0)
project(demoGlfwVulkan C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

find_package(imgui)

# 跨平台链接库配置
set(LINKER_LIBS glfw ${Vulkan_LIBRARIES} imgui::imgui)

if(WIN32)
    # Windows: Vulkan 库已在 ${Vulkan_LIBRARIES} 中，无需额外库
elseif(UNIX AND NOT APPLE)
    # Linux: 需要 X11 和其他窗口系统库
    list(APPEND LINKER_LIBS pthread GL dl X11 Xxf86vm Xrandr Xi)
elseif(APPLE)
    # macOS: Vulkan 支持有限，通常需要额外配置
    # macOS 通常不需要 X11
endif()

file(GLOB sources ./*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} ${LINKER_LIBS})
