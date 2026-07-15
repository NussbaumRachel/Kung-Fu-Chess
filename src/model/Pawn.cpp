#include "model/Pawn.hpp"
#include "model/Board.hpp"
#include <cmath>

Pawn::Pawn(Color color, Position startCell) : Piece(color, PieceType::Pawn, startCell) {}

bool Pawn::isValidMove(int fromRow,
                       int fromCol,
                       int toRow,
                       int toCol,
                       const Board& board) const
{
    const int direction = (color_ == Color::White) ? -1 : 1;
    const int startRow = (color_ == Color::White) ? board.rowCount() - 2 : 1;

    const int rowDiff = toRow - fromRow;
    const int colDiff = abs(toCol - fromCol);

    // ── תנועה ישרה ──
    if (toCol == fromCol)
    {
        // צעד אחד קדימה
        if (rowDiff == direction)
        {
            return board.isEmptyCell(toRow, toCol);
        }

        // צעד כפול — רק משורת הפתיחה, מסלול פנוי
        if (rowDiff == 2 * direction && fromRow == startRow)
        {
            int midRow = fromRow + direction;
            return board.isEmptyCell(midRow, toCol) &&
                   board.isEmptyCell(toRow, toCol);
        }
    }

    // ── לכידה באלכסון ──
    if (colDiff == 1 && rowDiff == direction)
    {
        return board.hasEnemyPiece(toRow, toCol, color_);
    }

    return false;
}
