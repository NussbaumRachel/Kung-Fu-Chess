#include "model/Queen.hpp"
#include "model/Rook.hpp"
#include "model/Bishop.hpp"
#include "model/Board.hpp"

Queen::Queen(Color color, Position startCell) : Piece(color, PieceType::Queen, startCell) {}

bool Queen::isValidMove(int fromRow, int fromCol, int toRow, int toCol, const Board& board) const
{
    // מלכה = צריח + רץ
    Rook rook(color_, cell_);
    Bishop bishop(color_, cell_);
    return rook.isValidMove(fromRow, fromCol, toRow, toCol, board) ||
           bishop.isValidMove(fromRow, fromCol, toRow, toCol, board);
}
