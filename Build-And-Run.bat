@echo off
setlocal
cd /d "%~dp0"
call Build-Debug.bat || exit /b 1
powershell -NoProfile -ExecutionPolicy Bypass -File .\Start-ChaseHQ.ps1
