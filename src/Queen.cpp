#include "Queen.hpp"
#include "Rook.hpp"
#include "Bishop.hpp"
#include "Board.hpp"

Queen::Queen(Color color) : Piece(color, PieceType::Queen) {}

bool Queen::isValidMove(int fromRow, int fromCol, int toRow, int toCol, const Board& board) const
{
    // מלכה = צריח + רץ
    Rook rook(color_);
    Bishop bishop(color_);
    return rook.isValidMove(fromRow, fromCol, toRow, toCol, board) ||
           bishop.isValidMove(fromRow, fromCol, toRow, toCol, board);
}
