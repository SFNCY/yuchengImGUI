set compiler_tool="Visual Studio 17 2022"

@REM set build_config_type=Debug
set build_config_type=Release
set current_abs_directory=%~dp0
set examples_directory=%current_abs_directory%/examples
set examples_build_directory=%examples_directory%/build
set thirdLibs_directory=%current_abs_directory%/3rdlibs

set data_path=%~dp0/data

@REM set PATH=%PATH%;^
@REM %thirdLibs_directory%\opencv-4.10.0\x64\vc17\bin;^
@REM %thirdLibs_directory%\ceres-solver-2.2.0\bin;

set CMAKE_EXE=%thirdLibs_directory%/cmake-3.31.4-windows-x86_64/bin/cmake.exe

%CMAKE_EXE% -E rm -rf %examples_build_directory%
%CMAKE_EXE% -E make_directory %examples_build_directory%
%CMAKE_EXE%                                                                      ^
-G %compiler_tool%                                                               ^
-S %examples_directory%                                                          ^
-B %examples_build_directory%                                                    ^
-DCMAKE_BUILD_TYPE=%build_config_type%                                           ^
-Dglfw3_DIR=%thirdLibs_directory%/install/lib/cmake/glfw3/                       ^
-Dimgui_DIR=%thirdLibs_directory%/install/lib/cmake/imgui

%CMAKE_EXE% --build %examples_build_directory% --config %build_config_type% --parallel 32

@REM call ./copydll.bat

%current_abs_directory%demo/build/opengl/Release/demoGlfwOpenGL.exe
@REM %current_abs_directory%demo/build/vulkan/Release/demoGlfwVulkan.exe