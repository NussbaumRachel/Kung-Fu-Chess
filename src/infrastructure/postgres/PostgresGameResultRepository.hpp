#pragma once

#include "game_result/IGameResultRepository.hpp"

#include <pqxx/pqxx>

#include <mutex>
#include <string>

class PostgresGameResultRepository final
    : public IGameResultRepository
{
public:
    explicit PostgresGameResultRepository(
        const std::string& connectionString
    );

    PostgresGameResultRepository(
        const PostgresGameResultRepository&
    ) = delete;

    PostgresGameResultRepository& operator=(
        const PostgresGameResultRepository&
    ) = delete;

    [[nodiscard]]
    GameResultId save(
        const GameResult& result
    ) override;

private:
    [[nodiscard]]
    static const char* winnerToDatabaseValue(
        GameWinner winner
    );

private:
    pqxx::connection connection_;
    std::mutex connectionMutex_;
};