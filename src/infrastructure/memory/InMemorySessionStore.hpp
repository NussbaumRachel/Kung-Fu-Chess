#pragma once

#include "session/ISessionStore.hpp"

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>

class InMemorySessionStore final
    : public ISessionStore
{
public:
    explicit InMemorySessionStore(
        std::chrono::seconds sessionTtl =
            std::chrono::seconds(120)
    );

    InMemorySessionStore(
        const InMemorySessionStore&
    ) = delete;

    InMemorySessionStore& operator=(
        const InMemorySessionStore&
    ) = delete;

    [[nodiscard]]
    bool tryAcquire(
        UserId userId,
        const std::string& sessionId
    ) override;

    [[nodiscard]]
    bool refresh(
        UserId userId,
        const std::string& sessionId
    ) override;

    void release(
        UserId userId,
        const std::string& sessionId
    ) override;

private:
    struct StoredSession
    {
        std::string sessionId;

        std::chrono::steady_clock::time_point
            expiresAt;
    };

private:
    std::mutex mutex_;

    std::chrono::seconds sessionTtl_;

    std::unordered_map<
        UserId,
        StoredSession
    > sessionsByUserId_;
};