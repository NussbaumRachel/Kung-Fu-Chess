#include "infrastructure/memory/InMemoryUserRepository.hpp"

#include <mutex>
#include <string>
#include <utility>

User InMemoryUserRepository::getOrCreateByUsername(
    const std::string& username)
{
    std::lock_guard<std::mutex> lock(mutex_);

    const auto existing =
        usersByUsername_.find(username);

    if (existing != usersByUsername_.end())
    {
        return existing->second;
    }

    User user(
        nextUserId_++,
        username
    );

    usersByUsername_.emplace(
        username,
        user
    );

    return user;
}