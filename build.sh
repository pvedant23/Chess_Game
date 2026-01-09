#!/bin/bash
# Simple build script for C++ Chess Game

echo "Building C++ Chess Game..."

# Compile the game
g++ -std=c++14 -Wall -Wextra -O2 main.cpp piece.cpp board.cpp game.cpp -o chess

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Run the game with: ./chess"
else
    echo "Build failed!"
    exit 1
fi
