#pragma once

#include "user/User.hpp"

#include <cstdint>

using GameResultId = std::int64_t;

enum class GameWinner
{
    White,
    Black
};

struct GameResult
{
    UserId whiteUserId;
    UserId blackUserId;
    GameWinner winner;
};