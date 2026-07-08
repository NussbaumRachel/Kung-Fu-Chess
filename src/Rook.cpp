#include "Rook.hpp"
#include "Board.hpp"

Rook::Rook(Color color) : Piece(color, PieceType::Rook) {}

bool Rook::isValidMove(int fromRow, int fromCol, int toRow, int toCol, const Board& board) const
{
    int dRow = toRow - fromRow;
    int dCol = toCol - fromCol;

    // צריח זז רק בקו ישר (אופקי או אנכי)
    if (dRow != 0 && dCol != 0)
        return false;

    // בדיקת חסימות בדרך
    int stepR = (dRow == 0) ? 0 : (dRow > 0 ? 1 : -1);
    int stepC = (dCol == 0) ? 0 : (dCol > 0 ? 1 : -1);

    int r = fromRow + stepR;
    int c = fromCol + stepC;
    while (r != toRow || c != toCol)
    {
        if (!board.isEmptyCell(r, c))
            return false;
        r += stepR;
        c += stepC;
    }

    return true;
}
