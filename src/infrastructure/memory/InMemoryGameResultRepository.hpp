#pragma once

#include "game_result/IGameResultRepository.hpp"

#include <mutex>
#include <vector>

class InMemoryGameResultRepository final
    : public IGameResultRepository
{
public:
    InMemoryGameResultRepository() = default;

    [[nodiscard]]
    GameResultId save(
        const GameResult& result
    ) override;

    [[nodiscard]]
    std::vector<GameResult> getAll() const;

private:
    mutable std::mutex mutex_;
    std::vector<GameResult> results_;
    GameResultId nextId_ = 1;
};