#include "Bishop.hpp"
#include "Board.hpp"
#include <cstdlib>

Bishop::Bishop(Color color) : Piece(color, PieceType::Bishop) {}

bool Bishop::isValidMove(int fromRow, int fromCol, int toRow, int toCol, const Board& board) const
{
    int rowDiff = abs(toRow - fromRow);
    int colDiff = abs(toCol - fromCol);

    // חייב להיות אלכסון
    if (rowDiff != colDiff)
        return false;

    if (!board.isPathClear(fromRow, fromCol, toRow, toCol))
        return false;

    if (board.hasFriendlyPiece(toRow, toCol, color_))
        return false;

    return true;
}
