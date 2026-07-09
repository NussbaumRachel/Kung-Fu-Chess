#include "Pawn.hpp"
#include "Board.hpp"
#include <cmath>
Pawn::Pawn(Color color, Position startCell) : Piece(color, PieceType::Pawn, startCell) {}

bool Pawn::isValidMove(int fromRow,
                       int fromCol,
                       int toRow,
                       int toCol,
                       const Board& board) const
{
    const int movementDirection =
        (color_ == Color::White) ? -1 : 1;

    const int rowDifference = toRow - fromRow;
    const int colDifference = abs(toCol - fromCol);


    // Move one square forward
    if (toCol == fromCol &&
        rowDifference == movementDirection)
    {
        return board.isEmptyCell(toRow, toCol);
    }


    // Capture diagonally
    if (colDifference == 1 &&
        rowDifference == movementDirection)
    {
        return board.hasEnemyPiece(toRow, toCol, color_);
    }


    return false;
}