#include "infrastructure/memory/InMemoryGameResultRepository.hpp"

#include <mutex>
#include <vector>

GameResultId InMemoryGameResultRepository::save(
    const GameResult& result)
{
    std::lock_guard<std::mutex> lock(
        mutex_
    );

    const GameResultId id = nextId_;

    ++nextId_;

    results_.push_back(result);

    return id;
}

std::vector<GameResult>
InMemoryGameResultRepository::getAll() const
{
    std::lock_guard<std::mutex> lock(
        mutex_
    );

    return results_;
}