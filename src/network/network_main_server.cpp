#include "network/GameServer.hpp"
#include "config/PieceSpeedConfig.hpp"
#include "infrastructure/postgres/PostgresConnectionConfig.hpp"
#include "infrastructure/postgres/PostgresUserRepository.hpp"
#include "model/Board.hpp"
#include "infrastructure/redis/RedisConnectionConfig.hpp"
#include "infrastructure/redis/RedisSessionStore.hpp"
#include "infrastructure/postgres/PostgresGameResultRepository.hpp"
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

int main(int argc, char* argv[])
{
    try
    {
        std::string assetsPath = "assets";

        if (argc >= 2)
        {
            assetsPath = argv[1];
        }

        if (!std::filesystem::exists(assetsPath))
        {
            const std::string alternativePath =
                std::filesystem::path(__FILE__)
                    .parent_path()
                    .string() +
                "/assets";

            if (
                std::filesystem::exists(
                    alternativePath
                )
            )
            {
                assetsPath = alternativePath;
            }
        }

        Board board({
            {
                "bR", "bN", "bB", "bQ",
                "bK", "bB", "bN", "bR"
            },
            {
                "bP", "bP", "bP", "bP",
                "bP", "bP", "bP", "bP"
            },
            {
                ".", ".", ".", ".",
                ".", ".", ".", "."
            },
            {
                ".", ".", ".", ".",
                ".", ".", ".", "."
            },
            {
                ".", ".", ".", ".",
                ".", ".", ".", "."
            },
            {
                ".", ".", ".", ".",
                ".", ".", ".", "."
            },
            {
                "wP", "wP", "wP", "wP",
                "wP", "wP", "wP", "wP"
            },
            {
                "wR", "wN", "wB", "wQ",
                "wK", "wB", "wN", "wR"
            }
        });

        PieceSpeedConfig speedConfig;

        speedConfig.load(
            assetsPath + "/pieces"
        );

        const PostgresConnectionConfig
            postgresConfig =
                PostgresConnectionConfig::
                    fromEnvironment();

        const std::string postgresConnectionString =
            postgresConfig.toConnectionString();

        PostgresUserRepository userRepository(
            postgresConnectionString
        );

        PostgresGameResultRepository
            gameResultRepository(
                postgresConnectionString
            );

        std::cout
            << "Connected to PostgreSQL"
            << std::endl;
            const RedisConnectionConfig redisConfig =
            RedisConnectionConfig::fromEnvironment();

        RedisSessionStore sessionStore(
            redisConfig
        );
        GameServer server(
            8080,
            std::move(board),
            std::move(speedConfig),
            userRepository,
            sessionStore,
            gameResultRepository
        );     
   server.run();
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "Server startup failed: "
            << exception.what()
            << std::endl;

        return 1;
    }

    return 0;
}