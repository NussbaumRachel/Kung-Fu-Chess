#pragma once

#include "user/User.hpp"

#include <string>

class IUserRepository
{
public:
    virtual ~IUserRepository() = default;

    [[nodiscard]]
    virtual User getOrCreateByUsername(
        const std::string& username
    ) = 0;
};