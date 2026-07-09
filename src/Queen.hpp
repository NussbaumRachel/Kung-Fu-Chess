#ifndef QUEEN_H
#define QUEEN_H

#include "Piece.hpp"

class Queen : public Piece
{
public:
    explicit Queen(Color color, Position startCell);
    bool isValidMove(int fromRow, int fromCol,
                     int toRow, int toCol,
                     const Board& board) const override;
};

#endif
