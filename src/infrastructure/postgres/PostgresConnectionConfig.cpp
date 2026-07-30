#include "infrastructure/postgres/PostgresConnectionConfig.hpp"

#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>

namespace
{
    std::string requireEnvironmentVariable(
        const char* variableName)
    {
        const char* value = std::getenv(variableName);

        if (
            value == nullptr ||
            *value == '\0'
        )
        {
            throw std::runtime_error(
                std::string(
                    "Missing required environment variable: "
                ) +
                variableName
            );
        }

        return value;
    }

    std::uint16_t parsePort(
        const std::string& value)
    {
        std::size_t parsedCharacters = 0;
        unsigned long parsedPort = 0;

        try
        {
            parsedPort = std::stoul(
                value,
                &parsedCharacters
            );
        }
        catch (const std::exception&)
        {
            throw std::runtime_error(
                "POSTGRES_PORT must be a valid number"
            );
        }

        if (
            parsedCharacters != value.size() ||
            parsedPort == 0 ||
            parsedPort >
                std::numeric_limits<std::uint16_t>::max()
        )
        {
            throw std::runtime_error(
                "POSTGRES_PORT must be between 1 and 65535"
            );
        }

        return static_cast<std::uint16_t>(
            parsedPort
        );
    }

    std::string quoteConnectionValue(
        const std::string& value)
    {
        std::string quoted;
        quoted.reserve(value.size() + 2);

        quoted.push_back('\'');

        for (const char character : value)
        {
            if (
                character == '\'' ||
                character == '\\'
            )
            {
                quoted.push_back('\\');
            }

            quoted.push_back(character);
        }

        quoted.push_back('\'');

        return quoted;
    }
}

PostgresConnectionConfig
PostgresConnectionConfig::fromEnvironment()
{
    const std::string portValue =
        requireEnvironmentVariable(
            "POSTGRES_PORT"
        );

    return {
        requireEnvironmentVariable(
            "POSTGRES_HOST"
        ),
        parsePort(portValue),
        requireEnvironmentVariable(
            "POSTGRES_DB"
        ),
        requireEnvironmentVariable(
            "POSTGRES_USER"
        ),
        requireEnvironmentVariable(
            "POSTGRES_PASSWORD"
        )
    };
}

std::string
PostgresConnectionConfig::toConnectionString() const
{
    return
        "host=" +
        quoteConnectionValue(host) +
        " port=" +
        std::to_string(port) +
        " dbname=" +
        quoteConnectionValue(database) +
        " user=" +
        quoteConnectionValue(username) +
        " password=" +
        quoteConnectionValue(password);
}