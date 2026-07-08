#include "Pawn.hpp"
#include "Board.hpp"

Pawn::Pawn(Color color) : Piece(color, PieceType::Pawn) {}

bool Pawn::isValidMove(int fromRow, int fromCol, int toRow, int toCol, const Board& board) const
{
    // רגלי — ימומש באיטרציה הבאה
    return false;
}
