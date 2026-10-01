@echo off
setlocal
cd /d "%~dp0"
if exist out\build\x64-Debug rmdir /s /q out\build\x64-Debug
if exist out\build\x64-Release rmdir /s /q out\build\x64-Release
call Build-Debug.bat || exit /b 1
call Build-Release.bat || exit /b 1
