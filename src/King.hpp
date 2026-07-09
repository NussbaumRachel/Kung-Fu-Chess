#ifndef KING_H
#define KING_H

#include "Piece.hpp"

class King : public Piece
{
public:
    explicit King(Color color, Position startCell);
    bool isValidMove(int fromRow, int fromCol,
                     int toRow, int toCol,
                     const Board& board) const override;
};

#endif
