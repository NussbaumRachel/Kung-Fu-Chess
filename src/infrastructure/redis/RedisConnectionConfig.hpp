#pragma once

#include <cstdint>
#include <string>

struct RedisConnectionConfig
{
    std::string host;
    std::uint16_t port;
    std::string password;
    int sessionTtlSeconds;

    [[nodiscard]]
    static RedisConnectionConfig fromEnvironment();
};