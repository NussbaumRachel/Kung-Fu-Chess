#pragma once

#include <cstdint>
#include <string>
#include <utility>

using UserId = std::int64_t;

struct User
{
    UserId id;
    std::string username;

    User(
        UserId userId,
        std::string userName
    )
        : id(userId),
          username(std::move(userName))
    {
    }
};