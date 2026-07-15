#include "model/Knight.hpp"
#include "model/Board.hpp"
#include <cstdlib>

Knight::Knight(Color color, Position startCell) : Piece(color, PieceType::Knight, startCell) {}

bool Knight::isValidMove(int fromRow, int fromCol, int toRow, int toCol, const Board& board) const
{
    int dRow = std::abs(toRow - fromRow);
    int dCol = std::abs(toCol - fromCol);

    // פרש: קפיצת L — 2+1 או 1+2
    return ((dRow == 2 && dCol == 1) || (dRow == 1 && dCol == 2)) 
    && (!board.hasFriendlyPiece(toRow, toCol, color_));
}
