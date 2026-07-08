#include "Bishop.hpp"
#include "Board.hpp"
#include <cstdlib>

Bishop::Bishop(Color color) : Piece(color, PieceType::Bishop) {}

bool Bishop::isValidMove(int fromRow, int fromCol, int toRow, int toCol, const Board& board) const
{
    int dRow = toRow - fromRow;
    int dCol = toCol - fromCol;

    // רץ זז רק באלכסון
    if (std::abs(dRow) != std::abs(dCol) || dRow == 0)
        return false;

    // בדיקת חסימות בדרך
    int stepR = dRow > 0 ? 1 : -1;
    int stepC = dCol > 0 ? 1 : -1;

    int r = fromRow + stepR;
    int c = fromCol + stepC;
    while (r != toRow)
    {
        if (!board.isEmptyCell(r, c))
            return false;
        r += stepR;
        c += stepC;
    }

    return true;
}
