#pragma once

#include "model/Piece.hpp"
#include <unordered_map>

class CooldownService
{
public:
    static constexpr int REST_AFTER_MOVE_MS = 2000;
    static constexpr int REST_AFTER_JUMP_MS = 500;

    void startResting(Piece* piece, int durationMs);
    void advanceRestTimers(int milliseconds);
    bool isResting(Piece* piece) const;

private:
    std::unordered_map<Piece*, int> restTimers_;
};
