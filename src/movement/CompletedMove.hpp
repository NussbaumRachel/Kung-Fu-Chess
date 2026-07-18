#pragma once

#include "model/Position.hpp"
#include "model/Constants.hpp"
#include <optional>

class Piece;

struct CompletedMove
{
    Piece* piece = nullptr;

    Position from;
    Position to;

    bool wasCancelled = false;
    bool wasIntercepted = false;

    bool wasStopped = false;        // חדש: המהלך נעצר מוקדם
    Position stoppedAtCell;         // חדש: איפה נעצר

    std::optional<PieceType> promoteTo;
};
