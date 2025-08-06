set compiler_tool="Visual Studio 17 2022"

@REM set build_config_type=Debug
set build_config_type=Release
set current_abs_directory=%~dp0
set build_directory=%current_abs_directory%build

set CMAKE_EXE=%thirdLibs_directory%/cmake-3.31.4-windows-x86_64/bin/cmake.exe

%CMAKE_EXE% -E rm -rf %build_directory%/CMakeCache.txt
@REM %CMAKE_EXE% -E rm -rf %build_directory%
@REM %CMAKE_EXE% -E make_directory %build_directory%
%CMAKE_EXE% -E rm -rf %install_directory%
%CMAKE_EXE% -G %compiler_tool%                                                         ^
-S %current_abs_directory%                                                             ^
-B %build_directory%                                                                   ^
-DCMAKE_BUILD_TYPE=%build_config_type%                                                 ^
-DCMAKE_INSTALL_PREFIX=%install_directory%


%CMAKE_EXE% --build %build_directory% --config %build_config_type% --parallel 32
%CMAKE_EXE% --install %build_directory% --config %build_config_type% --prefix %install_directory%

@REM call ./copydll.bat