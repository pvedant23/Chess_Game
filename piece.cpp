#include "chess.h"
#include <algorithm>

Position Position::fromAlgebraic(const std::string& algebraic){
    if (algebraic.length() != 2) return Position(-1,-1);
    int col = algebraic[0] - 'a';
    int row = algebraic[1] - '1';
    return Position(row, col);
}

std::string Position::toAlgebraic() const{
    if (!isValid()) return "";
    std::string result;
    result += (char)('a' + col);
    result += (char)('1' + row);
    return result;
}

// Helper function to add moves in a direction
std::vector<Move> addDirectionalMoves(const Position& pos, ChessBoard& board, int rowDir, int colDir, int maxSteps = 8){
    std::vector<Move> moves;
    Piece* piece = board.getPiece(pos);

    for(int step = 1;step <= maxSteps;step++){
        Position newPos(pos.row + step * rowDir, pos.col + step * colDir);

        if(!newPos.isValid()) break;
        Piece* targetPiece = board.getPiece(newPos);
        if(targetPiece == nullptr) {
            // Empty square
            moves.emplace_back(pos, newPos);
        } 
        else{
            // Occupied square
            if (targetPiece->color != piece->color) {
                // Enemy piece - can capture
                moves.emplace_back(pos, newPos);
            }
            break;
        }
    }
    return moves;
}

// Pawn implementation
std::vector<Move> Pawn::getPossibleMoves(const Position& pos, ChessBoard& board) const {
    std::vector<Move> moves;
    // std::cout << "Reached Pawn Start\n";
    int direction = (color == Color::WHITE) ? 1 : -1;
    int startRow = (color == Color::WHITE) ? 1 : 6;
    int promotionRow = (color == Color::WHITE) ? 7 : 0;

    // Forward move
    Position oneForward(pos.row + direction, pos.col);
    if(oneForward.isValid() && board.isEmpty(oneForward)){
        if(oneForward.row == promotionRow){
            // Promotion
            moves.emplace_back(pos, oneForward, PieceType::QUEEN);
            moves.emplace_back(pos, oneForward, PieceType::ROOK);
            moves.emplace_back(pos, oneForward, PieceType::BISHOP);
            moves.emplace_back(pos, oneForward, PieceType::KNIGHT);
        } else {
            moves.emplace_back(pos, oneForward);
        }

        // Two squares forward from starting position
        if (pos.row == startRow) {
            Position twoForward(pos.row + 2 * direction, pos.col);
            if (twoForward.isValid() && board.isEmpty(twoForward)) {
                moves.emplace_back(pos, twoForward);
            }
        }
    }

    // Diagonal captures
    for (int colOffset : {-1, 1}) {
        Position capturePos(pos.row + direction, pos.col + colOffset);
        if (capturePos.isValid()) {
            Piece* targetPiece = board.getPiece(capturePos);
            if (targetPiece && targetPiece->color != color) {
                if (capturePos.row == promotionRow) {
                    // Promotion capture
                    moves.emplace_back(pos, capturePos, PieceType::QUEEN);
                    moves.emplace_back(pos, capturePos, PieceType::ROOK);
                    moves.emplace_back(pos, capturePos, PieceType::BISHOP);
                    moves.emplace_back(pos, capturePos, PieceType::KNIGHT);
                } else {
                    moves.emplace_back(pos, capturePos);
                }
            }
            // En passant
            else if (board.isEnPassantTarget(capturePos)) {
                moves.emplace_back(pos, capturePos);
            }
        }
    }
    // std::cout << "Reached Pawn End\n";
    return moves;
}

// Rook implementation
std::vector<Move> Rook::getPossibleMoves(const Position& pos, ChessBoard& board) const {
    std::vector<Move> moves;
    // std::cout << "Reached Rook Start\n";
    // Horizontal and vertical directions
    int directions[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

    for (auto& dir : directions) {
        auto dirMoves = addDirectionalMoves(pos, board, dir[0], dir[1]);
        moves.insert(moves.end(), dirMoves.begin(), dirMoves.end());
    }
    // std::cout << "Reached Rook End\n";

    return moves;
}

// Knight implementation
std::vector<Move> Knight::getPossibleMoves(const Position& pos, ChessBoard& board) const {
    std::vector<Move> moves;
    // std::cout << "Reached Knight Start\n";
    // Knight moves: 8 possible L-shaped moves
    int knightMoves[8][2] = {
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
    };

    for (auto& move : knightMoves) {
        Position newPos(pos.row + move[0], pos.col + move[1]);

        if (newPos.isValid()) {
            Piece* targetPiece = board.getPiece(newPos);
            if (targetPiece == nullptr || targetPiece->color != color) {
                moves.emplace_back(pos, newPos);
            }
        }
    }
    // std::cout << "Reached Knight End\n";
    return moves;
}

// Bishop implementation
std::vector<Move> Bishop::getPossibleMoves(const Position& pos, ChessBoard& board) const {
    std::vector<Move> moves;
    // std::cout << "Reached Bishop Start\n";
    // Diagonal directions
    int directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    for (auto& dir : directions) {
        auto dirMoves = addDirectionalMoves(pos, board, dir[0], dir[1]);
        moves.insert(moves.end(), dirMoves.begin(), dirMoves.end());
    }
    // std::cout << "Reached Bishop End\n";
    return moves;
}

// Queen implementation
std::vector<Move> Queen::getPossibleMoves(const Position& pos, ChessBoard& board) const {
    std::vector<Move> moves;
    // std::cout << "Reached Queen Start\n";
    // Queen moves like both rook and bishop
    int directions[8][2] = {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0},    // Rook moves
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}   // Bishop moves
    };

    for (auto& dir : directions) {
        auto dirMoves = addDirectionalMoves(pos, board, dir[0], dir[1]);
        moves.insert(moves.end(), dirMoves.begin(), dirMoves.end());
    }
    // std::cout << "Reached Queen End\n";
    return moves;
}

// King implementation
std::vector<Move> King::getPossibleMoves(const Position& pos, ChessBoard& board) const {
    std::vector<Move> moves;
    // std::cout << "Reached King Start\n";
    // King moves: one square in any direction
    int directions[8][2] = {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    for (auto& dir : directions) {
        Position newPos(pos.row + dir[0], pos.col + dir[1]);

        if (newPos.isValid()) {
            Piece* targetPiece = board.getPiece(newPos);
            if (targetPiece == nullptr || targetPiece->color != color) {
                moves.emplace_back(pos, newPos);
            }
        }
    }
    
    // Castling
    if (!hasMoved) {
        // King-side castling
        if (board.canCastle(color, true)) {
            Position kingSidePos(pos.row, pos.col + 2);
            moves.emplace_back(pos, kingSidePos);
        }
        std::cout << "King Here\n";

        // Queen-side castling
        if (board.canCastle(color, false)) {
            Position queenSidePos(pos.row, pos.col - 2);
            moves.emplace_back(pos, queenSidePos);
        }
    }
    // std::cout << "Reached King End\n";
    return moves;
}
