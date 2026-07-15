#include "model/King.hpp"
#include "model/Board.hpp"
#include <cstdlib>

King::King(Color color, Position startCell) : Piece(color, PieceType::King, startCell) {}

bool King::isValidMove(int fromRow, int fromCol, int toRow, int toCol, const Board& board) const
{
    int dRow = std::abs(toRow - fromRow);
    int dCol = std::abs(toCol - fromCol);

    // מלך זז תא אחד בדיוק לכל כיוון
    return dRow <= 1 && dCol <= 1 && (dRow != 0 || dCol != 0) && (!board.hasFriendlyPiece(toRow, toCol, color_));
}


