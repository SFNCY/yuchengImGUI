clear
compiler_tool="Unix Makefiles"
# compiler_tool="Ninja"

build_config_type=Release
# build_config_type=Debug
current_abs_directory=$(dirname "$(readlink -f "$0")")
build_directory=${current_abs_directory}/build
install_directory=${current_abs_directory}/install

cmake -E rm -rf ${build_directory}
cmake -E make_directory ${build_directory}
cmake -E rm -rf ${install_directory}
cmake -G "${compiler_tool}"                                                                \
-S ${current_abs_directory}                                                                \
-B ${build_directory}                                                                      \
-DCMAKE_BUILD_TYPE=${build_config_type}                                                    \
-DCMAKE_INSTALL_PREFIX=${install_directory}

cmake --build ${build_directory} --config ${build_config_type} --parallel 32
cmake --install ${build_directory} --config ${build_config_type} --prefix ${install_directory}