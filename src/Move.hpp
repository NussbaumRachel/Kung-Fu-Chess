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
         int durationMs_
        );

    void update(int milliseconds);

    bool isFinished() const;

    Piece* getPiece() const;

    Position getFrom() const;
    Position getTo() const;
    bool isCancelled() const;
    void cancel();
    double getProgress() const;

private:
    Piece* piece_;
    Position from_;
    Position to_;
    bool cancelled_ = false;
    int remainingMs_;
    int durationMs_;
};

#endif