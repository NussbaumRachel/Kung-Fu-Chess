#ifndef BISHOP_H
#define BISHOP_H

#include "Piece.hpp"

class Bishop : public Piece
{
public:
    explicit Bishop(Color color);
    bool isValidMove(int fromRow, int fromCol,
                     int toRow, int toCol,
                     const Board& board) const override;
};

#endif
