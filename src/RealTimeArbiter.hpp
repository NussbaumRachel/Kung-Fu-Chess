#pragma once

#include "Move.hpp"
#include "Position.hpp"
#include <vector>

struct CompletedMove
{
    Position from;
    Position to;
    Piece* piece;
    bool wasCancelled;  // בוטל עקב התנגשות
};

class RealTimeArbiter
{
public:
    void startMove(Piece* piece, Position from, Position to, int durationMs);

    // קידום זמן לוגי — ללא sleep
    void advanceTime(int milliseconds);

    // איסוף מהלכים שהושלמו מאז ה-poll הקודם
    std::vector<CompletedMove> pollCompletedMoves();

    bool hasActiveMoves() const;

    bool isCellInvolved(int row, int col) const;

    const std::vector<Move>& getActiveMoves() const;

private:
    std::vector<Move> activeMoves_;
    std::vector<CompletedMove> completedMoves_;

    void resolveCollisions();
};
