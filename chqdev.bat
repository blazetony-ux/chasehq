@echo off
setlocal
cd /d "%~dp0"
set "BIN=tools\chqdev\bin\chqdev-windows-amd64.exe"
if /I "%PROCESSOR_ARCHITECTURE%"=="ARM64" set "BIN=tools\chqdev\bin\chqdev-windows-arm64.exe"
if not exist "%BIN%" (
  echo ERROR: bundled chqdev binary not found: %BIN%
  exit /b 1
)
"%BIN%" --root "%CD%" %*
exit /b %ERRORLEVEL%
