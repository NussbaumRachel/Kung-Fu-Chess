#include "Move.hpp"
#include "Piece.hpp"
#include "Position.hpp"
Move::Move(Piece* piece,
           Position from,
           Position to,
           int durationMs)
    :
    piece_(piece),
    from_(from),
    to_(to),
    remainingMs_(durationMs)
{
}


void Move::update(int milliseconds)
{
    remainingMs_ -= milliseconds;
    if (remainingMs_ < 0)
        remainingMs_ = 0;
}


bool Move::isFinished() const
{
    return remainingMs_ <= 0;
}


Piece* Move::getPiece() const
{
    return piece_;
}


Position Move::getFrom() const
{
    return from_;
}


Position Move::getTo() const
{
    return to_;
}