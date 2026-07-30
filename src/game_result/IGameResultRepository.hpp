#pragma once

#include "game_result/GameResult.hpp"

class IGameResultRepository
{
public:
    virtual ~IGameResultRepository() = default;

    [[nodiscard]]
    virtual GameResultId save(
        const GameResult& result
    ) = 0;
};