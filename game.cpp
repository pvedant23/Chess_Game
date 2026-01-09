#include "chess.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include<windows.h>
#ifdef _WIN32
    #define popen _popen
    #define pclose _pclose
#endif

ChessGame::ChessGame(bool enableCPU, Color cpuPlaysAs)
    : currentPlayer(Color::WHITE),
    gameOver(false),
    isCPUEnabled(enableCPU),
    cpuColor(cpuPlaysAs),
    humanColor(cpuPlaysAs == Color::WHITE ? Color::BLACK : Color::WHITE) {}


void ChessGame::startGame() {
    std::cout << "Welcome to C++ Chess Game!\n";
    std::cout << "Enter moves in algebraic notation (e.g., 'e2e4' or 'e2-e4')\n";
    std::cout << "Type 'quit' to exit the game\n\n";

    while (!gameOver) {
        board.displayBoard();

        std::string playerName = (currentPlayer == Color::WHITE) ? "White" : "Black";
        std::cout << playerName << " to move: ";

        playTurn();
        checkGameEnd();
    }

    std::cout << "Game Over: " << gameResult << std::endl;
}

void ChessGame::playTurn() {
    // If CPU is enabled and it's CPU's turn, let Stockfish move
    if (isCPUEnabled && currentPlayer == cpuColor) {
        std::cout << "CPU is thinking using Stockfish...\n";
        std::string fen = board.getFEN(currentPlayer);
        std::string bestMoveStr = getBestMoveFromStockfish(fen);

        if (bestMoveStr.length() < 4 || bestMoveStr.length() > 5){
            std::cout << "Invalid move from Stockfish: " << bestMoveStr << "\n";
            gameOver = true;
            gameResult = "Game ended due to engine error.";
            return;
        }

        Move move = parseAlgebraicNotation(bestMoveStr);
        std::cout << "Stockfish plays: " << bestMoveStr << "\n";
        std::cout << "Current player: " << ((currentPlayer == Color::WHITE) ? "White" : "Black") << "\n";

        board.movePiece(move);

        Color enemyColor = (currentPlayer == Color::WHITE) ? Color::BLACK : Color::WHITE;
        if (board.isInCheck(enemyColor)) {
            std::cout << "Check!\n";
        }

        switchPlayer();  //switch after CPU moves
        return;
    }

    //Regular user move
    std::string input;
    std::getline(std::cin, input);

    // Trim whitespace
    input.erase(input.begin(), std::find_if(input.begin(), input.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    input.erase(std::find_if(input.rbegin(), input.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), input.end());

    if (input == "quit" || input == "exit") {
        gameOver = true;
        gameResult = "Game terminated by user";
        return;
    }

    if (makeMove(input)) {
        switchPlayer();
    } else {
        std::cout << "Invalid move. Please try again.\n";
    }
}
std::string getBestMoveFromStockfish(const std::string& fen, const std::string& stockfishPath) {
    HANDLE hChildStd_IN_Rd = NULL;
    HANDLE hChildStd_IN_Wr = NULL;
    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT.
    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0)) {
        std::cerr << "Stdout pipe creation failed\n";
        return "";
    }
    // Ensure read handle to STDOUT is not inherited.
    SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);

    // Create a pipe for the child process's STDIN.
    if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0)) {
        std::cerr << "Stdin pipe creation failed\n";
        return "";
    }
    SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0);

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    STARTUPINFOA siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = hChildStd_OUT_Wr;
    siStartInfo.hStdInput = hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Start the child process.
    std::string cmdLine = stockfishPath;
    BOOL success = CreateProcessA(
        NULL,
        &cmdLine[0],
        NULL, NULL,
        TRUE,
        0,
        NULL, NULL,
        &siStartInfo,
        &piProcInfo
    );

    if (!success) {
        std::cerr << "CreateProcess failed\n";
        return "";
    }

    // Send UCI commands to Stockfish
    std::string commands =
        "uci\n"
        "isready\n"
        "ucinewgame\n"
        "position fen " + fen + "\n"
        "go depth 20\n";

    DWORD written;
    WriteFile(hChildStd_IN_Wr, commands.c_str(), commands.length(), &written, NULL);

    // Read Stockfish output
    char buffer[4096];
    DWORD bytesRead;
    std::string output;
    while (true) {
        BOOL result = ReadFile(hChildStd_OUT_Rd, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        if (!result || bytesRead == 0) break;
        buffer[bytesRead] = '\0';
        output += buffer;
        if (output.find("bestmove") != std::string::npos) break;
    }

    // Cleanup
    CloseHandle(hChildStd_IN_Wr);
    CloseHandle(hChildStd_IN_Rd);
    CloseHandle(hChildStd_OUT_Rd);
    CloseHandle(hChildStd_OUT_Wr);
    TerminateProcess(piProcInfo.hProcess, 0);
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    // Extract best move
    size_t pos = output.find("bestmove ");
    if (pos != std::string::npos) {
        size_t end = output.find('\n', pos);
        std::string moveLine = output.substr(pos, end - pos);
        std::istringstream iss(moveLine);
        std::string tag, move;
        iss >> tag >> move;
        return move;
    }

    return "";
}


bool ChessGame::makeMove(const std::string& algebraicMove) {
    try {
        Move move = parseAlgebraicNotation(algebraicMove);
        // std::cout << "Parsed move: " << move.from.toAlgebraic() << " to " << move.to.toAlgebraic() << std::endl;
        if (!move.from.isValid() || !move.to.isValid()) {
            std::cout << "Invalid move format. Use format like 'e2e4' or 'e2-e4'\n";
            return false;
        }

        Piece* piece = board.getPiece(move.from);
        if (!piece) {
            std::cout << "No piece at the specified position.\n";
            return false;
        }
        std::cout << "Piece at " << move.from.toAlgebraic() << " is " 
          << (piece->color == Color::WHITE ? "White" : "Black") << "\n";

        if (piece->color != currentPlayer) {
            std::cout << "That's not your piece!\n";
            return false;
        }

        // Check if the move is legal
        auto legalMoves = board.getAllLegalMoves(currentPlayer);
        bool isLegal = false;
        // std::cout << "Here\n";
        for (const auto& legalMove : legalMoves) {
            if (legalMove.from == move.from && legalMove.to == move.to) {
                // For pawn promotion, check if promotion piece matches
                if (piece->type == PieceType::PAWN && 
                    ((piece->color == Color::WHITE && move.to.row == 7) ||
                     (piece->color == Color::BLACK && move.to.row == 0))) {
                    if (legalMove.promotionPiece == move.promotionPiece) {
                        isLegal = true;
                        break;
                    }
                } else {
                    isLegal = true;
                    break;
                }
            }
        }

        if (!isLegal) {
            std::cout << "Illegal move.\n";
            return false;
        }

        // Make the move
        board.movePiece(move);

        // Check for check
        Color enemyColor = (currentPlayer == Color::WHITE) ? Color::BLACK : Color::WHITE;
        if (board.isInCheck(enemyColor)) {
            std::cout << "Check!\n";
        }

        return true;

    } catch (const std::exception& e) {
        std::cout << "Error parsing move: " << e.what() << std::endl;
        return false;
    }
}

Move ChessGame::parseAlgebraicNotation(const std::string& notation) const {
    std::string cleanNotation = notation;

    // Remove common separators
    size_t dashPos = cleanNotation.find('-');
    if (dashPos != std::string::npos) {
        cleanNotation.erase(dashPos, 1);
    }
    std::cout << "Parsing move: " << cleanNotation << std::endl;
    // Remove spaces
    cleanNotation.erase(std::remove(cleanNotation.begin(), cleanNotation.end(), ' '), 
                       cleanNotation.end());

    std:: cout << "Cleaned notation: " << cleanNotation << std::endl;
    // Convert to lowercase for parsing
    std::transform(cleanNotation.begin(), cleanNotation.end(), cleanNotation.begin(), ::tolower);

    if (cleanNotation.length() < 4) {
        throw std::invalid_argument("Move too short");
    }


    // Parse from position
    std::string fromStr = cleanNotation.substr(0, 2);
    Position from = Position::fromAlgebraic(fromStr);

    // Parse to position
    std::string toStr = cleanNotation.substr(2, 2);
    Position to = Position::fromAlgebraic(toStr);
    std:: cout << fromStr << " to " << toStr << std::endl;
    // Check for promotion
    PieceType promotionPiece = PieceType::QUEEN;  // Default promotion
    if (cleanNotation.length() >= 5) {
        char promoChar = cleanNotation[4];
        switch (promoChar) {
            case 'q': promotionPiece = PieceType::QUEEN; break;
            case 'r': promotionPiece = PieceType::ROOK; break;
            case 'b': promotionPiece = PieceType::BISHOP; break;
            case 'n': promotionPiece = PieceType::KNIGHT; break;
            default: promotionPiece = PieceType::QUEEN; break;
        }
    }

    return Move(from, to, promotionPiece);
}

void ChessGame::checkGameEnd() {
    if (board.isCheckmate(currentPlayer)) {
        gameOver = true;
        std::string winner = (currentPlayer == Color::WHITE) ? "Black" : "White";
        gameResult = winner + " wins by checkmate!";
    } else if (board.isStalemate(currentPlayer)) {
        gameOver = true;
        gameResult = "Draw by stalemate!";
    }
    // more things can be implemented here
}
