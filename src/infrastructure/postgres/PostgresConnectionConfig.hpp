#pragma once

#include <cstdint>
#include <string>

struct PostgresConnectionConfig
{
    std::string host;
    std::uint16_t port;
    std::string database;
    std::string username;
    std::string password;

    [[nodiscard]]
    static PostgresConnectionConfig fromEnvironment();

    [[nodiscard]]
    std::string toConnectionString() const;
};