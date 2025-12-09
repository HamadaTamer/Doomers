@echo off
echo ========================================
echo DOOMERS - Create Distribution Package
echo ========================================

set DIST_DIR=Doomers_Distribution
set TEMPLATE_DIR=%~dp0

echo Creating distribution folder...
if exist "%DIST_DIR%" rmdir /s /q "%DIST_DIR%"
mkdir "%DIST_DIR%"

echo Copying executable...
copy "Debug\OpenGL3DTemplate.exe" "%DIST_DIR%\Doomers.exe"

echo Copying required DLLs...
REM Assimp DLLs
copy "assimp-vc140-mt.dll" "%DIST_DIR%\"
copy "Debug\assimp-vc140-mt.dll" "%DIST_DIR%\" 2>nul
copy "Debug\assimp-vc143-mt.dll" "%DIST_DIR%\" 2>nul

REM GLUT/GLEW DLLs
copy "Debug\glut32.dll" "%DIST_DIR%\"
copy "glew32.dll" "%DIST_DIR%\"

REM Copy Visual C++ Runtime DLLs from system (if available)
echo Copying Visual C++ Runtime DLLs...
copy "C:\Windows\System32\msvcp140.dll" "%DIST_DIR%\" 2>nul
copy "C:\Windows\System32\vcruntime140.dll" "%DIST_DIR%\" 2>nul
copy "C:\Windows\System32\vcruntime140_1.dll" "%DIST_DIR%\" 2>nul
copy "C:\Windows\System32\vcruntime140d.dll" "%DIST_DIR%\" 2>nul
copy "C:\Windows\System32\msvcp140d.dll" "%DIST_DIR%\" 2>nul
copy "C:\Windows\System32\ucrtbased.dll" "%DIST_DIR%\" 2>nul

echo Copying resources folder...
xcopy "res" "%DIST_DIR%\res" /E /I /Q

echo.
echo ========================================
echo Distribution package created!
echo ========================================
echo.
echo Location: %TEMPLATE_DIR%%DIST_DIR%
echo.
echo IMPORTANT: If users still get DLL errors, they need to install:
echo   Visual C++ Redistributable for Visual Studio 2022
echo   Download from: https://aka.ms/vs/17/release/vc_redist.x86.exe
echo.
echo Contents:
dir "%DIST_DIR%" /b
echo.
pause
