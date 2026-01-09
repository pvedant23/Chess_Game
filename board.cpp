#include "chess.h"
#include <iostream>
#include <iomanip>

ChessBoard::ChessBoard() : enPassantAvailable(false) {
    // Initialize empty board
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            board[row][col] = nullptr;
        }
    }
    setupInitialPosition();
}

ChessBoard::ChessBoard(const ChessBoard& other) : enPassantAvailable(false) {
    copyFrom(other);
}

ChessBoard& ChessBoard::operator=(const ChessBoard& other) {
    if (this != &other) {
        copyFrom(other);
    }
    return *this;
}

void ChessBoard::copyFrom(const ChessBoard& other) {
    // Copy board state
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (other.board[row][col]) {
                board[row][col] = other.board[row][col]->clone();
            } else {
                board[row][col] = nullptr;
            }
        }
    }

    // Copy en passant state
    enPassantTarget = other.enPassantTarget;
    enPassantAvailable = other.enPassantAvailable;
}

void ChessBoard::setupInitialPosition() {
    // Clear the board first
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            board[row][col] = nullptr;
        }
    }

    // Set up white pieces (row 0 and 1)
    board[0][0] = std::make_unique<Rook>(Color::WHITE);
    board[0][1] = std::make_unique<Knight>(Color::WHITE);
    board[0][2] = std::make_unique<Bishop>(Color::WHITE);
    board[0][3] = std::make_unique<Queen>(Color::WHITE);
    board[0][4] = std::make_unique<King>(Color::WHITE);
    board[0][5] = std::make_unique<Bishop>(Color::WHITE);
    board[0][6] = std::make_unique<Knight>(Color::WHITE);
    board[0][7] = std::make_unique<Rook>(Color::WHITE);

    for (int col = 0; col < 8; col++) {
        board[1][col] = std::make_unique<Pawn>(Color::WHITE);
    }

    // Set up black pieces (row 6 and 7)
    for (int col = 0; col < 8; col++) {
        board[6][col] = std::make_unique<Pawn>(Color::BLACK);
    }

    board[7][0] = std::make_unique<Rook>(Color::BLACK);
    board[7][1] = std::make_unique<Knight>(Color::BLACK);
    board[7][2] = std::make_unique<Bishop>(Color::BLACK);
    board[7][3] = std::make_unique<Queen>(Color::BLACK);
    board[7][4] = std::make_unique<King>(Color::BLACK);
    board[7][5] = std::make_unique<Bishop>(Color::BLACK);
    board[7][6] = std::make_unique<Knight>(Color::BLACK);
    board[7][7] = std::make_unique<Rook>(Color::BLACK);

    enPassantAvailable = false;
}

void ChessBoard::displayBoard() const {
    std::cout << "\n  +---+---+---+---+---+---+---+---+\n";

    for (int row = 7; row >= 0; row--) {
        std::cout << (row + 1) << " |";
        for (int col = 0; col < 8; col++) {
            if (board[row][col]) {
                std::cout << " " << board[row][col]->getSymbol() << " |";
            } else {
                std::cout << "   |";
            }
        }
        std::cout << "\n  +---+---+---+---+---+---+---+---+\n";
    }

    std::cout << "    a   b   c   d   e   f   g   h\n\n";
}

Piece* ChessBoard::getPiece(const Position& pos) const {
    if (!pos.isValid()) return nullptr;
    return board[pos.row][pos.col].get();
}

bool ChessBoard::isEmpty(const Position& pos) const {
    if (!pos.isValid()) return false;
    return board[pos.row][pos.col] == nullptr;
}

bool ChessBoard::movePiece(const Move& move) {
    if (!move.from.isValid() || !move.to.isValid()) return false;

    Piece* piece = getPiece(move.from);
    if (!piece) return false;

    // Clear en passant flag at the start of each move
    bool wasEnPassant = enPassantAvailable;
    Position oldEnPassantTarget = enPassantTarget;
    clearEnPassant();

    // Handle special moves
    bool isEnPassantCapture = false;
    bool isCastling = false;
    bool isPromotion = false;

    // Check for en passant capture
    if (piece->type == PieceType::PAWN && move.to == oldEnPassantTarget && wasEnPassant) {
        isEnPassantCapture = true;
    }

    // Check for castling
    if (piece->type == PieceType::KING && abs(move.to.col - move.from.col) == 2) {
        isCastling = true;
    }

    // Check for promotion
    if (piece->type == PieceType::PAWN && 
        ((piece->color == Color::WHITE && move.to.row == 7) ||
         (piece->color == Color::BLACK && move.to.row == 0))) {
        isPromotion = true;
    }

    // Perform the move
    std::unique_ptr<Piece> capturedPiece = std::move(board[move.to.row][move.to.col]);
    board[move.to.row][move.to.col] = std::move(board[move.from.row][move.from.col]);

    // Mark piece as moved
    board[move.to.row][move.to.col]->hasMoved = true;

    // Handle special move cases
    if (isEnPassantCapture) {
        // Remove the captured pawn
        int capturedPawnRow = (piece->color == Color::WHITE) ? move.to.row - 1 : move.to.row + 1;
        board[capturedPawnRow][move.to.col] = nullptr;
    }

    if (isCastling) {
        // Move the rook
        int rookFromCol = (move.to.col > move.from.col) ? 7 : 0;  // King-side or queen-side
        int rookToCol = (move.to.col > move.from.col) ? 5 : 3;

        board[move.from.row][rookToCol] = std::move(board[move.from.row][rookFromCol]);
        board[move.from.row][rookToCol]->hasMoved = true;
    }

    if (isPromotion) {
        // Replace pawn with promoted piece
        Color color = board[move.to.row][move.to.col]->color;
        switch (move.promotionPiece) {
            case PieceType::QUEEN:
                board[move.to.row][move.to.col] = std::make_unique<Queen>(color);
                break;
            case PieceType::ROOK:
                board[move.to.row][move.to.col] = std::make_unique<Rook>(color);
                break;
            case PieceType::BISHOP:
                board[move.to.row][move.to.col] = std::make_unique<Bishop>(color);
                break;
            case PieceType::KNIGHT:
                board[move.to.row][move.to.col] = std::make_unique<Knight>(color);
                break;
            default:
                board[move.to.row][move.to.col] = std::make_unique<Queen>(color);
                break;
        }
        board[move.to.row][move.to.col]->hasMoved = true;
    }

    // Set en passant target if pawn moved two squares
    if (piece->type == PieceType::PAWN && abs(move.to.row - move.from.row) == 2) {
        int enPassantRow = (move.from.row + move.to.row) / 2;
        setEnPassant(Position(enPassantRow, move.from.col));
    }

    return true;
}

bool ChessBoard::isSquareAttacked(const Position& pos, Color attackingColor, bool castling_check = false) const {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Piece* piece = board[row][col].get();
            if (piece && piece->color == attackingColor) {
                Position piecePos(row, col);

                // For pawns, check diagonal attacks specifically
                if (piece->type == PieceType::PAWN) {
                    int direction = (attackingColor == Color::WHITE) ? 1 : -1;
                    Position leftAttack(row + direction, col - 1);
                    Position rightAttack(row + direction, col + 1);

                    if (leftAttack == pos || rightAttack == pos) {
                        return true;
                    }
                } else if (castling_check && piece->type == PieceType::KING) {
                    // For king, check if the square is attacked by any piece
                    // King can only move one square, so we check all adjacent squares
                    int directions[8][2] = {
                        {0, 1}, {0, -1}, {1, 0}, {-1, 0},
                        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
                    };

                    for (const auto& dir : directions) {
                        Position adjacentPos(piecePos.row + dir[0], piecePos.col + dir[1]);
                        if (adjacentPos == pos) {
                            return true;
                        }
                    }
                } else if (piece->type == PieceType::KING && !castling_check)
                {
                    /* code */
                } else {
                    // For other pieces, get their possible moves
                    // We need to be careful not to cause infinite recursion
                    // Create a temporary non-const reference
                    ChessBoard* nonConstThis = const_cast<ChessBoard*>(this);
                    auto moves = piece->getPossibleMoves(piecePos, *nonConstThis);

                    for (const auto& move : moves) {
                        if (move.to == pos) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

Position ChessBoard::getKingPosition(Color color) const {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Piece* piece = board[row][col].get();
            if (piece && piece->type == PieceType::KING && piece->color == color) {
                return Position(row, col);
            }
        }
    }
    return Position(-1, -1);  // King not found (shouldn't happen)
}

bool ChessBoard::isInCheck(Color kingColor) const {
    Position kingPos = getKingPosition(kingColor);
    if (!kingPos.isValid()) return false;

    Color enemyColor = (kingColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    return isSquareAttacked(kingPos, enemyColor);
}

bool ChessBoard::wouldBeInCheck(const Move& move, Color kingColor) const {
    // Create a copy of the board to test the move
    ChessBoard tempBoard = *this;

    // Make the move on the temporary board
    tempBoard.movePiece(move);

    // Check if the king would be in check
    return tempBoard.isInCheck(kingColor);
}
std::vector<Move> ChessBoard::getAllLegalMoves(Color color) const {
    std::vector<Move> legalMoves;

    for (int row = 0; row <8; row++) { 
        for (int col = 0; col <8; col++) {
            Piece* piece = board[row][col].get();
            if (piece && piece->color == color) {
                Position piecePos(row, col);
                ChessBoard* nonConstThis = const_cast<ChessBoard*>(this);
                // std::cout << "Here"<<row<<col<<"\n";
                auto possibleMoves = piece->getPossibleMoves(piecePos, *nonConstThis);
                for (const auto& move : possibleMoves) {
                    if (!wouldBeInCheck(move, color)) {
                        legalMoves.push_back(move);
                    }
                }
                // std::cout << "Here3\n";
            }
        }
    }

    return legalMoves;
}

bool ChessBoard::isCheckmate(Color color) const {
    return isInCheck(color) && getAllLegalMoves(color).empty();
}

bool ChessBoard::isStalemate(Color color) const {
    return !isInCheck(color) && getAllLegalMoves(color).empty();
}

std::string ChessBoard::getFEN(Color currentPlayer) const {
    std::string fen = "";

    for (int row = 7; row >= 0; row--) {
        int empty = 0;
        for (int col = 0; col < 8; col++) {
            Piece* piece = board[row][col].get();
            if (piece == nullptr) {
                empty++;
            } else {
                if (empty > 0) {
                    fen += std::to_string(empty);
                    empty = 0;
                }
                char symbol = piece->getSymbol();
                fen += symbol;
            }
        }
        if (empty > 0) fen += std::to_string(empty);
        if (row > 0) fen += "/";
    }

    // Side to move
    fen += " ";
    fen += (currentPlayer == Color::WHITE) ? "w" : "b";

    // Simplify: no castling, en passant, halfmove clock, fullmove number
    fen += " - - 0 1";

    return fen;
}


bool ChessBoard::canCastle(Color color, bool kingSide) const {
    Position kingPos = getKingPosition(color);
    if (!kingPos.isValid()) return false;

    Piece* king = getPiece(kingPos);
    if (!king || king->hasMoved) return false;

    // Check if king is in check
    if (isInCheck(color)) return false;

    int row = kingPos.row;
    int rookCol = kingSide ? 7 : 0;

    // Check if rook exists and hasn't moved
    Piece* rook = getPiece(Position(row, rookCol));
    if (!rook || rook->type != PieceType::ROOK || rook->hasMoved) return false;

    // Check if squares between king and rook are empty
    int startCol = std::min(kingPos.col, rookCol) + 1;
    int endCol = std::max(kingPos.col, rookCol);

    for (int col = startCol; col < endCol; col++) {
        if (!isEmpty(Position(row, col))) return false;
    }

    // Check if squares the king passes through are not under attack
    Color enemyColor = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;
    int kingDirection = kingSide ? 1 : -1;

    for (int i = 1; i <= 2; i++) {
        Position checkPos(row, kingPos.col + i * kingDirection);
        if (isSquareAttacked(checkPos, enemyColor,true)) return false;
    }

    return true;
}
