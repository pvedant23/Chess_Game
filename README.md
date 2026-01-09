# Chess_Game
It is a fully functional, AI-integrated chess game built from scratch using C++ and Object-Oriented Programming principles. The game supports both Player vs Player and Player vs Computer (CPU) modes with Stockfish as the chess engine.

Stockfish Setup Instructions
To enable Player vs CPU mode using the Stockfish engine, follow these steps:

1: Download Stockfish
Go to https://stockfishchess.org/download/ and download the appropriate version for your operating system.

2: Place the Executable
After extracting the download, place the Stockfish executable file in the same directory as your chess program (e.g., next to chess.exe or chess).

3: Set the Path in Code
Open "chess.h" and update the Stockfish path in the following function:

std::string getBestMoveFromStockfish(const std::string& fen, const std::string& stockfishPath = "stockfish.exe");
If the executable is not in the same folder, replace "stockfish.exe" with the full path to the Stockfish executable file.

4: Run the Game
Compile and run the game. Choose the "Player vs CPU" option to play against Stockfish.
