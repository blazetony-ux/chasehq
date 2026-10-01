@echo off
setlocal
cd /d "%~dp0"
call Find-CMake.bat || exit /b 1
"%CMAKE_EXE%" --preset x64-debug || exit /b 1
"%CMAKE_EXE%" --build --preset x64-debug --target sprite_gfx_tests || exit /b 1
"%CTEST_EXE%" --test-dir out\build\x64-Debug -C Debug -R "^sprite_gfx_tests$" --output-on-failure
exit /b %ERRORLEVEL%
