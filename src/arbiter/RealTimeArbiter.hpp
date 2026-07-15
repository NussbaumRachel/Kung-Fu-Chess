#pragma once

#include "movement/Move.hpp"
#include "model/Position.hpp"
#include "movement/CompletedMove.hpp"
#include <vector>



// מבנה פנימי לכלי קופץ
struct JumpEntry
{
    Piece* piece;
    Position cell;
    int remainingMs;
};

class RealTimeArbiter
{
public:
    void startMove(Piece* piece, Position from, Position to, int durationMs);

    // הפעלת קפיצה — 1000ms, הכלי נשאר במקום
    void startJump(Piece* piece, Position cell, int durationMs);

    // קידום זמן לוגי — ללא sleep
    void advanceTime(int milliseconds);

    // איסוף מהלכים שהושלמו מאז ה-poll הקודם
    std::vector<CompletedMove> pollCompletedMoves();

    bool hasActiveMoves() const;
    bool hasActiveJumps() const;

    bool isCellInvolved(int row, int col) const;

    // בודק אם הכלי בתא הוא המקור של מהלך/קפיצה פעילים
    // (לעומת isCellInvolved שבודק גם targets — כלי יכול לשבת בתא
    //  שהוא target של מהלך אויב, אבל הוא עצמו לא מעורב)
    bool isPieceInvolved(int row, int col) const;

    const std::vector<Move>& getActiveMoves() const;
    const std::vector<JumpEntry>& getActiveJumps() const;

    // איפוס jumps שהושלמו (נקרא ע"י GameEngine)
    std::vector<JumpEntry> pollCompletedJumps();

private:
    std::vector<Move> activeMoves_;
    std::vector<CompletedMove> completedMoves_;

    std::vector<JumpEntry> activeJumps_;
    std::vector<JumpEntry> completedJumps_;

    void resolveCollisions();
    void resolveJumpInterceptions();
};
