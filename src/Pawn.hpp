#ifndef PAWN_H
#define PAWN_H

#include "Piece.hpp"

class Pawn : public Piece
{
public:
    explicit Pawn(Color color);
    bool isValidMove(int fromRow, int fromCol,
                     int toRow, int toCol,
                     const Board& board) const override;
};

#endif
