#include "infrastructure/postgres/PostgresGameResultRepository.hpp"

#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <string>

PostgresGameResultRepository::
PostgresGameResultRepository(
    const std::string& connectionString)
    : connection_(connectionString)
{
    if (!connection_.is_open())
    {
        throw std::runtime_error(
            "Failed to open PostgreSQL connection "
            "for game results"
        );
    }
}

GameResultId PostgresGameResultRepository::save(
    const GameResult& result)
{
    std::lock_guard<std::mutex> lock(
        connectionMutex_
    );

    pqxx::work transaction(connection_);

    const pqxx::result queryResult =
        transaction.exec_params(
            R"SQL(
                INSERT INTO game_results
                (
                    white_user_id,
                    black_user_id,
                    winner
                )
                VALUES ($1, $2, $3)
                RETURNING id
            )SQL",
            result.whiteUserId,
            result.blackUserId,
            winnerToDatabaseValue(
                result.winner
            )
        );

    if (queryResult.size() != 1)
    {
        throw std::runtime_error(
            "PostgreSQL did not return "
            "the game result ID"
        );
    }

    const GameResultId gameResultId =
        queryResult[0]["id"].as<std::int64_t>();

    transaction.commit();

    return gameResultId;
}

const char*
PostgresGameResultRepository::
winnerToDatabaseValue(
    GameWinner winner)
{
    switch (winner)
    {
        case GameWinner::White:
            return "white";

        case GameWinner::Black:
            return "black";
    }

    throw std::invalid_argument(
        "Unsupported game winner"
    );
}