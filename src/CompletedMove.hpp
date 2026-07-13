#pragma once

#include "Position.hpp"

class Piece;

struct CompletedMove
{
    Piece* piece = nullptr;

    Position from;
    Position to;

    bool wasCancelled = false;
    bool wasIntercepted = false;   // בוטל ע"י יירוט קפיצה — piece נלכד
};