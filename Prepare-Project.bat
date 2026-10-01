@echo off
setlocal EnableExtensions
cd /d "%~dp0"
set "CHECK_ARG="
set "ROM_SOURCE_ARG="
set "SDL_SOURCE_ARG="
:parse
if "%~1"=="" goto dispatch
if /I "%~1"=="--check" set "CHECK_ARG=-Check"& shift & goto parse
if /I "%~1"=="--roms" set "ROM_SOURCE_ARG=%~2"& shift & shift & goto parse
if /I "%~1"=="--sdl" set "SDL_SOURCE_ARG=%~2"& shift & shift & goto parse
echo ERROR: Unknown option %~1
exit /b 2
:dispatch
if defined ROM_SOURCE_ARG goto have_rom
if defined SDL_SOURCE_ARG goto only_sdl
goto none
:have_rom
if defined SDL_SOURCE_ARG goto both
powershell -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Prepare-Project.ps1" %CHECK_ARG% -Roms "%ROM_SOURCE_ARG%"
exit /b %ERRORLEVEL%
:only_sdl
powershell -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Prepare-Project.ps1" %CHECK_ARG% -Sdl "%SDL_SOURCE_ARG%"
exit /b %ERRORLEVEL%
:both
powershell -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Prepare-Project.ps1" %CHECK_ARG% -Roms "%ROM_SOURCE_ARG%" -Sdl "%SDL_SOURCE_ARG%"
exit /b %ERRORLEVEL%
:none
powershell -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Prepare-Project.ps1" %CHECK_ARG%
exit /b %ERRORLEVEL%
