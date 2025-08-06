
clear
compiler_tool="Unix Makefiles"
# compiler_tool="Ninja"

build_config_type=Release
# build_config_type=Debug

# 指定临时环境变量，链接动态库
# export LD_LIBRARY_PATH=/home/yucheng/3rdlib/opencv-4.10.0/install/lib/:$LD_LIBRARY_PATH
# export LD_LIBRARY_PATH=/home/yucheng/yuchengGit/yuchengVTK/3rdlibs/VTK-9.2.6/lib:$LD_LIBRARY_PATH
# export LD_LIBRARY_PATH=/home/yucheng/yuchengGit/AIRVision3D/install/lib:$LD_LIBRARY_PATH
# echo ${LD_LIBRARY_PATH}

# cd examples
current_abs_directory=$(dirname "$(readlink -f "$0")")
examples_directory=${current_abs_directory}/demo
examples_build_directory=${examples_directory}/build

cmake -E rm -rf ${examples_build_directory}
cmake -E make_directory ${examples_build_directory}
cmake -G "${compiler_tool}"                                          \
-S ${examples_directory}                                             \
-B ${examples_build_directory}                                       \
-DCMAKE_BUILD_TYPE=${build_config_type}                              \
-Dglfw3_DIR=${current_abs_directory}/install/lib/cmake/glfw3/        \
-Dimgui_DIR=${current_abs_directory}/install/lib/cmake/imgui

cmake --build ${examples_build_directory} --config ${build_config_type} --parallel 32

./demo/build/opengl/demoGlfwOpenGL
# ./demo/build/vulkan/demoGlfwVulkan