#include "RealTimeArbiter.hpp"
#include "Piece.hpp"

void RealTimeArbiter::startMove(Piece* piece,
                                Position from, Position to,
                                int durationMs)
{
    activeMoves_.emplace_back(piece, from, to, durationMs);
}

void RealTimeArbiter::advanceTime(int milliseconds)
{
    completedMoves_.clear();

    resolveCollisions();

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
}

std::vector<CompletedMove> RealTimeArbiter::pollCompletedMoves()
{
    return std::move(completedMoves_);
}

bool RealTimeArbiter::hasActiveMoves() const
{
    return !activeMoves_.empty();
}

bool RealTimeArbiter::isCellInvolved(int row, int col) const
{
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
    return false;
}

const std::vector<Move>& RealTimeArbiter::getActiveMoves() const
{
    return activeMoves_;
}

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
                    // המאוחר מבוטל — זה עם durationMs הנותר הקטן יותר (שמסיים מוקדם) מנצח
                    if (first.getRemainingMs() <= second.getRemainingMs())
                    {
                        second.cancel();
                    }
                    else
                    {
                        first.cancel();
                    }
                }
            }
        }
    }
}
