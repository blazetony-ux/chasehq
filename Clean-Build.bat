@echo off
setlocal
cd /d "%~dp0"
if exist out\build rmdir /s /q out\build
echo Build output removed.
