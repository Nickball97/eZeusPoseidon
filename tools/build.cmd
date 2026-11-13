@echo off
rem Builds the developer version of the game: Release, with the test
rem shortcuts, the crash reporter and the safety net (EZEUS_DEV_CHEATS).
rem
rem   tools\build.cmd
rem
rem The build folder is "eZeus-build" next to the repository folder, or
rem the folder named in EZEUS_BUILD. Compiler messages go to build.log in
rem that folder; search it for "error C" or "error LNK" when the build
rem fails, and for "warning C" for warnings.
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul || exit /b 1
for %%I in ("%~dp0..") do set SRC=%%~fI
set BLD=%EZEUS_BUILD%
if "%BLD%"=="" for %%I in ("%~dp0..\..\eZeus-build") do set BLD=%%~fI
if not exist "%BLD%\build.ninja" (
  cmake -S "%SRC%" -B "%BLD%" -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev || exit /b 1
)
rem -k 0: keep going after errors so one run reports all of them
cmake -S "%SRC%" -B "%BLD%" -DEZEUS_DEV_CHEATS=ON >nul || exit /b 1
cmake --build "%BLD%" --parallel -- -k 0 > "%BLD%\build.log" 2>&1
if errorlevel 1 (
  echo BUILD FAILED, see %BLD%\build.log
  exit /b 1
)
echo BUILD OK
