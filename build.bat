@echo off
echo Building C++ Chess Game...

g++ -std=c++14 -Wall -Wextra -O2 main.cpp piece.cpp board.cpp game.cpp -o chess.exe

if %errorlevel% equ 0 (
    echo Build successful!
    echo Run the game with: chess.exe
) else (
    echo Build failed!
    pause
    exit /b 1
)

pause
