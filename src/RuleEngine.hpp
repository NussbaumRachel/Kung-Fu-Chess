#pragma once

#include "Position.hpp"
#include "types.hpp"
#include <string>

class Board;

struct MoveValidation
{
    bool isValid;
    std::string reason;  // "ok", "outside_board", "empty_source",
                         // "friendly_destination", "illegal_piece_move"
};

class RuleEngine
{
public:
    MoveValidation validateMove(const Board& board,
                                Position from, Position to,
                                Color playerColor) const;
};
