#include "Rook.hpp"
#include "Board.hpp"

Rook::Rook(Color color) : Piece(color, PieceType::Rook) {}

bool Rook::isValidMove(int fromRow, int fromCol, int toRow, int toCol, const Board& board) const
{
     if (fromRow != toRow && fromCol != toCol)
        return false;

    if (fromRow == toRow && fromCol == toCol)
        return false;

    if (!board.isPathClear(fromRow, fromCol, toRow, toCol))
        return false;

    if (board.hasFriendlyPiece(toRow, toCol, color_))
        return false;

    return true;
}
