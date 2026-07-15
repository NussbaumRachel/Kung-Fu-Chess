#ifndef KNIGHT_H
#define KNIGHT_H

#include "model/Piece.hpp"

class Knight : public Piece
{
public:
    explicit Knight(Color color, Position startCell);
    bool isValidMove(int fromRow, int fromCol,
                     int toRow, int toCol,
                     const Board& board) const override;
};

#endif
