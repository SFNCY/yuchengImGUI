set compiler_tool="Visual Studio 17 2022"

@REM set build_config_type=Debug
set build_config_type=Release
set current_abs_directory=%~dp0
@REM set demo_directory=%current_abs_directory%/examples
@REM set demo_build_directory=%demo_directory%/build
set demo_directory=%current_abs_directory%/demo
set demo_build_directory=%demo_directory%/build
set thirdLibs_directory=%current_abs_directory%/3rdlibs

set data_path=%~dp0/data

@REM set PATH=%PATH%;^
@REM %thirdLibs_directory%\opencv-4.10.0\x64\vc17\bin;^
@REM %thirdLibs_directory%\ceres-solver-2.2.0\bin;

set CMAKE_EXE=%thirdLibs_directory%/cmake-3.31.4-windows-x86_64/bin/cmake.exe

@REM %CMAKE_EXE% -E rm -rf %demo_build_directory%
%CMAKE_EXE% -E make_directory %demo_build_directory%
%CMAKE_EXE%                                                                      ^
-G %compiler_tool%                                                               ^
-S %demo_directory%                                                              ^
-B %demo_build_directory%                                                        ^
-DCMAKE_BUILD_TYPE=%build_config_type%                                           ^
-Dglfw3_DIR=%current_abs_directory%/install/lib/cmake/glfw3/                       ^
-Dimgui_DIR=%current_abs_directory%/install/lib/cmake/imgui

%CMAKE_EXE% --build %demo_build_directory% --config %build_config_type% --parallel 32

@REM call ./copydll.bat

@REM %current_abs_directory%demo/build/opengl/Release/demoGlfwOpenGL.exe
%current_abs_directory%demo/build/vulkan/Release/demoGlfwVulkan.exe