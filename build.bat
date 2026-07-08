@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvars64.bat"

if %ERRORLEVEL% NEQ 0 exit /b 1

cd /d C:\Kung-Fu-Chess

if not exist build mkdir build

for %%f in (
Piece
King
Queen
Rook
Bishop
Knight
Pawn
Board
BoardParser
Game
main
) do (
    echo Compiling %%f.cpp
    cl /nologo /EHsc /std:c++17 /I src /c src\%%f.cpp /Fo:build\%%f.obj || exit /b 1
)

echo Linking...

cl /nologo /EHsc build\*.obj /Fe:build\kungfuchess.exe || exit /b 1

echo.
echo Build completed.

build\kungfuchess.exe