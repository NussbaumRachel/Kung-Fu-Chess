#include "Knight.hpp"
#include "Board.hpp"
#include <cstdlib>

Knight::Knight(Color color) : Piece(color, PieceType::Knight) {}

bool Knight::isValidMove(int fromRow, int fromCol, int toRow, int toCol, const Board& board) const
{
    int dRow = std::abs(toRow - fromRow);
    int dCol = std::abs(toCol - fromCol);

    // פרש: קפיצת L — 2+1 או 1+2
    return (dRow == 2 && dCol == 1) || (dRow == 1 && dCol == 2);
}
