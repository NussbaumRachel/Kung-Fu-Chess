#include "Pawn.hpp"
#include "Board.hpp"

Pawn::Pawn(Color color) : Piece(color, PieceType::Pawn) {}

bool Pawn::isValidMove(int fromRow, int fromCol,
                       int toRow, int toCol,
                       const Board& board) const
{
    int direction = (color_ == Color::White) ? -1 : 1;

    int rowDiff = toRow - fromRow;
    int colDiff = abs(toCol - fromCol);

    // תנועה קדימה
    if (toCol == fromCol &&
        rowDiff == direction &&
        board.isEmptyCell(toRow, toCol))
    {
        return true;
    }

    // אכילה באלכסון
    if (colDiff == 1 &&
        rowDiff == direction &&
        board.hasEnemyPiece(toRow, toCol, color_))
    {
        return true;
    }

    return false;
}
