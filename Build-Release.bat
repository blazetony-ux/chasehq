@echo off
setlocal EnableDelayedExpansion
cd /d "%~dp0"
echo [1/7] Preparing project and local dependencies...
call Prepare-Project.bat || exit /b 1
echo [2/7] Checking CMake...
call Find-CMake.bat || exit /b 1
echo [3/7] Project validation complete.
echo [4/7] Configuring Release...
"%CMAKE_EXE%" --preset x64-release || exit /b 1
echo [5/7] Building Release...
"%CMAKE_EXE%" --build --preset x64-release --parallel || exit /b 1
echo [6/7] Running launcher process/argument smoke...
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File ".\Start-ChaseHQResearch.ps1" -NoShell -ValidateLaunchOnly -Checkpoint ".\checkpoints\stage1-gameplay-2064.chqstate" || exit /b 1
echo [7/7] Running tests...
echo BUILD COMPILE: PASS
set "CTEST_LOG=%TEMP%\chq-ctest-release-%RANDOM%.log"
"%CTEST_EXE%" --test-dir out\build\x64-Release -C Release --output-on-failure > "%CTEST_LOG%" 2>&1
set "CTEST_RC=%ERRORLEVEL%"
type "%CTEST_LOG%"
if not "%CTEST_RC%"=="0" (
  echo.
  echo BUILD COMPILE: PASS
  echo TESTS: FAIL
  del "%CTEST_LOG%" >nul 2>&1
  exit /b 1
)
del "%CTEST_LOG%" >nul 2>&1
echo.
echo BUILD COMPILE: PASS
echo TESTS: PASS
echo BUILD SUCCESSFUL
exit /b 0
