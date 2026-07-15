#ifndef ROOK_H
#define ROOK_H

#include "model/Piece.hpp"

class Rook : public Piece
{
public:
    explicit Rook(Color color, Position startCell);
    bool isValidMove(int fromRow, int fromCol,
                     int toRow, int toCol,
                     const Board& board) const override;
};

#endif
