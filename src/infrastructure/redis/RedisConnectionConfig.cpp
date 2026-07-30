#include "infrastructure/redis/RedisConnectionConfig.hpp"

#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>

namespace
{
    std::string requireEnvironmentVariable(
        const char* name)
    {
        const char* value = std::getenv(name);

        if (value == nullptr || *value == '\0')
        {
            throw std::runtime_error(
                std::string(
                    "Missing environment variable: "
                ) + name
            );
        }

        return value;
    }

    int parsePositiveInteger(
        const std::string& value,
        const char* variableName)
    {
        std::size_t parsedCharacters = 0;
        long parsedValue = 0;

        try
        {
            parsedValue = std::stol(
                value,
                &parsedCharacters
            );
        }
        catch (const std::exception&)
        {
            throw std::runtime_error(
                std::string(
                    "Invalid integer in environment variable: "
                ) + variableName
            );
        }

        if (
            parsedCharacters != value.size() ||
            parsedValue <= 0 ||
            parsedValue >
                std::numeric_limits<int>::max()
        )
        {
            throw std::runtime_error(
                std::string(
                    "Invalid positive integer in environment variable: "
                ) + variableName
            );
        }

        return static_cast<int>(parsedValue);
    }
}

RedisConnectionConfig
RedisConnectionConfig::fromEnvironment()
{
    RedisConnectionConfig config;

    config.host =
        requireEnvironmentVariable(
            "REDIS_HOST"
        );

    const int parsedPort =
        parsePositiveInteger(
            requireEnvironmentVariable(
                "REDIS_PORT"
            ),
            "REDIS_PORT"
        );

    if (
        parsedPort >
        std::numeric_limits<std::uint16_t>::max()
    )
    {
        throw std::runtime_error(
            "REDIS_PORT is outside the valid port range"
        );
    }

    config.port =
        static_cast<std::uint16_t>(
            parsedPort
        );

    config.password =
        requireEnvironmentVariable(
            "REDIS_PASSWORD"
        );

    config.sessionTtlSeconds =
        parsePositiveInteger(
            requireEnvironmentVariable(
                "REDIS_SESSION_TTL_SECONDS"
            ),
            "REDIS_SESSION_TTL_SECONDS"
        );

    return config;
}