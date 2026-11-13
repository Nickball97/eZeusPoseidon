@echo off
rem Builds sgextract.exe with MSVC, using the zlib sources that the eZeus
rem CMake build downloads (SDL_image's external/zlib).
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul || exit /b 1
set Z=%~dp0..\..\..\eZeus-build\_deps\sdl2_image-src\external\zlib
if not exist "%Z%\zlib.h" (
  echo zlib not found at %Z% - build eZeus once first
  exit /b 1
)
set OUT=%~1
if "%OUT%"=="" set OUT=%~dp0
cl /nologo /O2 /EHsc /std:c++17 /I"%Z%" "%~dp0sgextract.cpp" "%Z%\adler32.c" "%Z%\compress.c" "%Z%\crc32.c" "%Z%\deflate.c" "%Z%\trees.c" "%Z%\zutil.c" /Fe:"%OUT%\sgextract.exe" /Fo:"%OUT%\\" || exit /b 1
