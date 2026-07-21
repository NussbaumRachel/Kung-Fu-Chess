#pragma once

#include "movement/Move.hpp"
#include "model/Position.hpp"
#include <vector>

class Board;
struct JumpEntry;

class CollisionResolver
{
public:
    /// Run all three collision phases in order.
    /// Mutates activeMoves/activeJumps in-place.
    /// May mutate Board (takeCell for stationary captures).
    void resolve(std::vector<Move>& activeMoves,
                 std::vector<JumpEntry>& activeJumps,
                 Board& board);

private:
    void resolveTargetCollisions(std::vector<Move>& activeMoves);
    void resolveJumpInterceptions(std::vector<Move>& activeMoves,
                                  std::vector<JumpEntry>& activeJumps);
    void resolvePathCollisions(std::vector<Move>& activeMoves,
                               Board& board);
};
