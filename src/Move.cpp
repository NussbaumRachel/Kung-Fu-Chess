#include "Move.hpp"
#include "Piece.hpp"
#include "Position.hpp"
Move::Move(Piece* piece,
           Position from,
           Position to,
           int durationMs)
    : piece_(piece),
      from_(from),
      to_(to),
      remainingMs_(durationMs),
      durationMs_(durationMs)
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

void Move::cancel()
{
    cancelled_ = true;
}

void Move::setIntercepted()
{
    intercepted_ = true;
}

bool Move::isIntercepted() const
{
    return intercepted_;
}

bool Move::isCancelled() const
{
    return cancelled_;
}
double Move::getProgress() const
{
    return 1.0 -
           static_cast<double>(remainingMs_) /
           durationMs_;
}