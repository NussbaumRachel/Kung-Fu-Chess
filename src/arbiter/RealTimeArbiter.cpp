#include "arbiter/RealTimeArbiter.hpp"
#include "model/Piece.hpp"

// ──────────────── Moves ────────────────

void RealTimeArbiter::startMove(Piece* piece,
                                Position from, Position to,
                                int durationMs)
{
    activeMoves_.emplace_back(piece, from, to, durationMs);
}

// ──────────────── Jumps ────────────────

void RealTimeArbiter::startJump(Piece* piece, Position cell, int durationMs)
{
    activeJumps_.push_back({piece, cell, durationMs});
}

// ──────────────── advanceTime ────────────────

void RealTimeArbiter::advanceTime(int milliseconds)
{
    completedMoves_.clear();
    completedJumps_.clear();

    resolveCollisions();

    // ── יירוטים: קופץ לוכד אויב שמגיע לתא שלו ──
    // חייב לרוץ לפני שה-jumps מתעדכנים, כדי שקפיצות שעדיין פעילות
    // (גם אם הן עומדות להסתיים באותו tick) יוכלו ליירט מהלכים.
    resolveJumpInterceptions();

    // ── עדכון Moves ──
    for (auto it = activeMoves_.begin(); it != activeMoves_.end(); )
    {
        it->update(milliseconds);

        if (it->isCancelled())
        {
            CompletedMove cm;
            cm.from = it->getFrom();
            cm.to = it->getTo();
            cm.piece = it->getPiece();
            cm.wasCancelled = true;
            cm.wasIntercepted = it->isIntercepted();
            completedMoves_.push_back(cm);
            it = activeMoves_.erase(it);
        }
        else if (it->isFinished())
        {
            CompletedMove cm;
            cm.from = it->getFrom();
            cm.to = it->getTo();
            cm.piece = it->getPiece();
            cm.wasCancelled = false;
            completedMoves_.push_back(cm);
            it = activeMoves_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // ── עדכון Jumps ──
    for (auto it = activeJumps_.begin(); it != activeJumps_.end(); )
    {
        it->remainingMs -= milliseconds;
        if (it->remainingMs <= 0)
        {
            it->remainingMs = 0;
            completedJumps_.push_back(*it);
            it = activeJumps_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// ──────────────── Poll ────────────────

std::vector<CompletedMove> RealTimeArbiter::pollCompletedMoves()
{
    return std::move(completedMoves_);
}

std::vector<JumpEntry> RealTimeArbiter::pollCompletedJumps()
{
    return std::move(completedJumps_);
}

// ──────────────── Queries ────────────────

bool RealTimeArbiter::hasActiveMoves() const
{
    return !activeMoves_.empty();
}

bool RealTimeArbiter::hasActiveJumps() const
{
    return !activeJumps_.empty();
}

bool RealTimeArbiter::isCellInvolved(int row, int col) const
{
    // בדיקת moves (from/to)
    for (const Move& move : activeMoves_)
    {
        Position from = move.getFrom();
        Position to = move.getTo();
        if ((from.row == row && from.col == col) ||
            (to.row == row && to.col == col))
        {
            return true;
        }
    }
    // בדיקת jumps
    for (const JumpEntry& jump : activeJumps_)
    {
        if (jump.cell.row == row && jump.cell.col == col)
        {
            return true;
        }
    }
    return false;
}

bool RealTimeArbiter::isPieceInvolved(int row, int col) const
{
    // בודק רק מקורות (from) — לא targets.
    // כלי בתא (row,col) מעורב רק אם הוא יצא לדרך
    for (const Move& move : activeMoves_)
    {
        Position from = move.getFrom();
        if (from.row == row && from.col == col)
            return true;
    }
    // jump — תמיד source = target cell, אבל גם כאן בודקים
    for (const JumpEntry& jump : activeJumps_)
    {
        if (jump.cell.row == row && jump.cell.col == col)
            return true;
    }
    return false;
}

const std::vector<Move>& RealTimeArbiter::getActiveMoves() const
{
    return activeMoves_;
}

const std::vector<JumpEntry>& RealTimeArbiter::getActiveJumps() const
{
    return activeJumps_;
}

// ──────────────── Collisions ────────────────

void RealTimeArbiter::resolveCollisions()
{
    for (size_t i = 0; i < activeMoves_.size(); ++i)
    {
        for (size_t j = i + 1; j < activeMoves_.size(); ++j)
        {
            Move& first = activeMoves_[i];
            Move& second = activeMoves_[j];

            if (first.isCancelled() || second.isCancelled())
                continue;

            Piece* firstPiece = first.getPiece();
            Piece* secondPiece = second.getPiece();

            if (firstPiece == nullptr || secondPiece == nullptr)
                continue;

            // התנגשות בין כלים מיריבים באותו יעד
            if (firstPiece->getColor() != secondPiece->getColor())
            {
                if (first.getTo() == second.getTo())
                {
                    if (first.getRemainingMs() <= second.getRemainingMs())
                        second.cancel();
                    else
                        first.cancel();
                }
            }
        }
    }
}

// ──────────────── Jump Interceptions ────────────────

void RealTimeArbiter::resolveJumpInterceptions()
{
    for (auto& jump : activeJumps_)
    {
        for (auto& move : activeMoves_)
        {
            if (move.isCancelled() || move.isFinished())
                continue;

            // בודק האם אויב בתנועה מגיע לתא של הקופץ
            if (move.getTo() == jump.cell &&
                move.getPiece() != nullptr &&
                move.getPiece()->getColor() != jump.piece->getColor())
            {
                // הקופץ לוכד את האויב — האויב מבוטל
                move.cancel();
                move.setIntercepted();
            }
        }
    }
}
