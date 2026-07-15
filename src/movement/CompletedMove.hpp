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
    bool wasIntercepted = false;   // בוטל ע"י יירוט קפיצה — piece נלכד

    std::optional<PieceType> promoteTo;  // None = Queen (ברירת מחדל)
};