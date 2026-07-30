#pragma once

#include "user/IUserRepository.hpp"

#include <mutex>
#include <string>
#include <unordered_map>

class InMemoryUserRepository final
    : public IUserRepository
{
public:
    InMemoryUserRepository() = default;

    [[nodiscard]]
    User getOrCreateByUsername(
        const std::string& username
    ) override;

private:
    std::mutex mutex_;

    std::unordered_map<
        std::string,
        User
    > usersByUsername_;

    UserId nextUserId_ = 1;
};