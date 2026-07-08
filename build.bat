@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: vcvars64.bat failed
    exit /b 1
)
cd /d C:\Kung-Fu-Chess
if not exist build mkdir build
echo ===== Piece.cpp =====
cl /nologo /EHsc /std:c++17 /I src /c src\Piece.cpp /Fo:build\Piece.obj 2>&1 || exit /b 1
echo ===== King.cpp =====
cl /nologo /EHsc /std:c++17 /I src /c src\King.cpp /Fo:build\King.obj 2>&1 || exit /b 1
echo ===== Rook.cpp =====
cl /nologo /EHsc /std:c++17 /I src /c src\Rook.cpp /Fo:build\Rook.obj 2>&1 || exit /b 1
echo ===== Bishop.cpp =====
cl /nologo /EHsc /std:c++17 /I src /c src\Bishop.cpp /Fo:build\Bishop.obj 2>&1 || exit /b 1
echo ===== Queen.cpp =====
cl /nologo /EHsc /std:c++17 /I src /c src\Queen.cpp /Fo:build\Queen.obj 2>&1 || exit /b 1
echo ===== Knight.cpp =====
cl /nologo /EHsc /std:c++17 /I src /c src\Knight.cpp /Fo:build\Knight.obj 2>&1 || exit /b 1
echo ===== Pawn.cpp =====
cl /nologo /EHsc /std:c++17 /I src /c src\Pawn.cpp /Fo:build\Pawn.obj 2>&1 || exit /b 1
echo ===== Board.cpp =====
cl /nologo /EHsc /std:c++17 /I src /c src\Board.cpp /Fo:build\Board.obj 2>&1 || exit /b 1
echo ===== BoardParser.cpp =====
cl /nologo /EHsc /std:c++17 /I src /c src\BoardParser.cpp /Fo:build\BoardParser.obj 2>&1 || exit /b 1
echo ===== Game.cpp =====
cl /nologo /EHsc /std:c++17 /I src /c src\Game.cpp /Fo:build\Game.obj 2>&1 || exit /b 1
echo ===== main.cpp =====
cl /nologo /EHsc /std:c++17 /I src /c src\main.cpp /Fo:build\main.obj 2>&1 || exit /b 1
echo ===== Linking =====
cl /nologo /EHsc build\Piece.obj build\King.obj build\Rook.obj build\Bishop.obj build\Queen.obj build\Knight.obj build\Pawn.obj build\Board.obj build\BoardParser.obj build\Game.obj build\main.obj /Fe:build\kungfuchess.exe 2>&1 || exit /b 1
echo ===== Done =====
echo.
echo ===== Running kungfuchess.exe =====
echo.
build\kungfuchess.exe
