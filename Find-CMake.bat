@echo off
setlocal
set "FOUND_CMAKE="
if exist "%CD%\tools\cmake\bin\cmake.exe" set "FOUND_CMAKE=%CD%\tools\cmake\bin\cmake.exe"
if not defined FOUND_CMAKE for /f "delims=" %%I in ('where cmake.exe 2^>nul') do if not defined FOUND_CMAKE set "FOUND_CMAKE=%%I"
if not defined FOUND_CMAKE if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" for /f "usebackq delims=" %%I in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do if exist "%%I\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" set "FOUND_CMAKE=%%I\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if not defined FOUND_CMAKE (
  echo ERROR: CMake not found. Install CMake or Visual Studio Build Tools with C++/CMake support.
  endlocal & exit /b 1
)
for %%I in ("%FOUND_CMAKE%") do set "FOUND_CTEST=%%~dpIctest.exe"
endlocal & set "CMAKE_EXE=%FOUND_CMAKE%" & set "CTEST_EXE=%FOUND_CTEST%" & exit /b 0
