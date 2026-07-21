#include "game_engine/CooldownService.hpp"

void CooldownService::startResting(Piece* piece, int durationMs)
{
    if (piece)
        restTimers_[piece] = durationMs;
}

void CooldownService::advanceRestTimers(int milliseconds)
{
    for (auto it = restTimers_.begin(); it != restTimers_.end(); )
    {
        it->second -= milliseconds;
        if (it->second <= 0)
        {
            if (it->first)
                it->first->setState(PieceState::Idle);
            it = restTimers_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool CooldownService::isResting(Piece* piece) const
{
    return restTimers_.find(piece) != restTimers_.end();
}
