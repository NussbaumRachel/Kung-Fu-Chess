#ifndef MOVE_HPP
#define MOVE_HPP

#include "Position.hpp"

class Piece;

class Move
{
public:
    Move(Piece* piece,
         Position from,
         Position to,
         int durationMs);

    void update(int milliseconds);

    bool isFinished() const;

    Piece* getPiece() const;

    Position getFrom() const;
    Position getTo() const;

private:
    Piece* piece_;

    Position from_;
    Position to_;

    int remainingMs_;
};

#endif