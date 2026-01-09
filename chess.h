#ifndef CHESS_H
#define CHESS_H

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <cstdlib>
#include <sstream>
#include <fstream>

enum class Color { WHITE, BLACK };
enum class PieceType { PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING };



struct Position {
    int row, col;

    
    Position(int r = 0, int c = 0) : row(r), col(c) {}

    bool isValid() const {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }

    // Convert algebraic notation (e.g., "e4") to Position
    static Position fromAlgebraic(const std::string& algebraic);

    // Convert Position to algebraic notation
    std::string toAlgebraic() const;
};

struct Move {
    Position from, to;
    PieceType promotionPiece = PieceType::QUEEN;

    Move(Position f, Position t) : from(f), to(t) {}
    Move(Position f, Position t, PieceType promo) : from(f), to(t), promotionPiece(promo) {}
};

class Piece {
public:
    Color color;
    PieceType type;
    bool hasMoved;

    Piece(Color c, PieceType t) : color(c), type(t), hasMoved(false) {}
    virtual ~Piece() = default;

    virtual std::vector<Move> getPossibleMoves(const Position& pos, class ChessBoard& board) const = 0;
    virtual char getSymbol() const = 0;
    virtual std::unique_ptr<Piece> clone() const = 0;
};

class Pawn : public Piece {
public:
    Pawn(Color c) : Piece(c, PieceType::PAWN) {}
    std::vector<Move> getPossibleMoves(const Position& pos, ChessBoard& board) const override;
    char getSymbol() const override { return (color == Color::WHITE) ? 'P' : 'p'; }
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Pawn>(*this); }
};

class Rook : public Piece {
public:
    Rook(Color c) : Piece(c, PieceType::ROOK) {}
    std::vector<Move> getPossibleMoves(const Position& pos, ChessBoard& board) const override;
    char getSymbol() const override { return (color == Color::WHITE) ? 'R' : 'r'; }
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Rook>(*this); }
};

class Knight : public Piece {
public:
    Knight(Color c) : Piece(c, PieceType::KNIGHT) {}
    std::vector<Move> getPossibleMoves(const Position& pos, ChessBoard& board) const override;
    char getSymbol() const override { return (color == Color::WHITE) ? 'N' : 'n'; }
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Knight>(*this); }
};

class Bishop : public Piece {
public:
    Bishop(Color c) : Piece(c, PieceType::BISHOP) {}
    std::vector<Move> getPossibleMoves(const Position& pos, ChessBoard& board) const override;
    char getSymbol() const override { return (color == Color::WHITE) ? 'B' : 'b'; }
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Bishop>(*this); }
};

class Queen : public Piece {
public:
    Queen(Color c) : Piece(c, PieceType::QUEEN) {}
    std::vector<Move> getPossibleMoves(const Position& pos, ChessBoard& board) const override;
    char getSymbol() const override { return (color == Color::WHITE) ? 'Q' : 'q'; }
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Queen>(*this); }
};

class King : public Piece {
public:
    King(Color c) : Piece(c, PieceType::KING) {}
    std::vector<Move> getPossibleMoves(const Position& pos, ChessBoard& board) const override;
    char getSymbol() const override { return (color == Color::WHITE) ? 'K' : 'k'; }
    std::unique_ptr<Piece> clone() const override { return std::make_unique<King>(*this); }
};

class ChessBoard {
private:
    std::unique_ptr<Piece> board[8][8];
    Position enPassantTarget;
    bool enPassantAvailable;

public:
    ChessBoard();
    ChessBoard(const ChessBoard& other);
    ChessBoard& operator=(const ChessBoard& other);
    ~ChessBoard() = default;

    // Helper method for deep copying
    void copyFrom(const ChessBoard& other);

    void setupInitialPosition();
    void displayBoard() const;

    Piece* getPiece(const Position& pos) const;
    bool isEmpty(const Position& pos) const;
    bool movePiece(const Move& move);

    bool isSquareAttacked(const Position& pos, Color attackingColor,bool castling_check) const;
    bool isInCheck(Color kingColor) const;
    bool wouldBeInCheck(const Move& move, Color kingColor) const;

    std::vector<Move> getAllLegalMoves(Color color) const;
    bool isCheckmate(Color color) const;
    bool isStalemate(Color color) const;

    Position getKingPosition(Color color) const;
    bool canCastle(Color color, bool kingSide) const;

    std::string getFEN(Color currentPlayer) const;
    void setEnPassant(const Position& pos) { enPassantTarget = pos; enPassantAvailable = true; }
    void clearEnPassant() { enPassantAvailable = false; }
    bool isEnPassantTarget(const Position& pos) const { return enPassantAvailable && enPassantTarget == pos; }
};

class ChessGame {
private:
    ChessBoard board;
    Color currentPlayer;
    bool gameOver;
    std::string gameResult;

    bool isCPUEnabled;
    Color cpuColor;
    Color humanColor;


public:
     ChessGame(bool enableCPU, Color cpuPlaysAs = Color::BLACK);

    void startGame();
    void playTurn();
    bool makeMove(const std::string& algebraicMove);
    void switchPlayer() { currentPlayer = (currentPlayer == Color::WHITE) ? Color::BLACK : Color::WHITE; }
    Color getCurrentPlayer() const { return currentPlayer; }
    bool isGameOver() const { return gameOver; }
    std::string getGameResult() const { return gameResult; }
    private:
    Move parseAlgebraicNotation(const std::string& notation) const;
    void checkGameEnd();
    
};

std::string getBestMoveFromStockfish(const std::string& fen, const std::string& stockfishPath = "stockfish.exe");
#endif // CHESS_H
