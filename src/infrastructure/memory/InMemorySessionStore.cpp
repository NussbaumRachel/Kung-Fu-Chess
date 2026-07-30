#include "infrastructure/memory/InMemorySessionStore.hpp"

#include <chrono>
#include <mutex>
#include <string>
#include <utility>

InMemorySessionStore::InMemorySessionStore(
    std::chrono::seconds sessionTtl
)
    : sessionTtl_(sessionTtl)
{
}

bool InMemorySessionStore::tryAcquire(
    UserId userId,
    const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(
        mutex_
    );

    const auto now =
        std::chrono::steady_clock::now();

    const auto expiresAt =
        now + sessionTtl_;

    const auto existing =
        sessionsByUserId_.find(userId);

    if (existing == sessionsByUserId_.end())
    {
        sessionsByUserId_.emplace(
            userId,
            StoredSession{
                sessionId,
                expiresAt
            }
        );

        return true;
    }

    /*
     * Session שפג אינו חוסם התחברות חדשה.
     */
    if (existing->second.expiresAt <= now)
    {
        existing->second = StoredSession{
            sessionId,
            expiresAt
        };

        return true;
    }

    /*
     * הפעולה idempotent עבור אותו session.
     */
    if (existing->second.sessionId == sessionId)
    {
        existing->second.expiresAt =
            expiresAt;

        return true;
    }

    return false;
}

bool InMemorySessionStore::refresh(
    UserId userId,
    const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(
        mutex_
    );

    const auto existing =
        sessionsByUserId_.find(userId);

    if (existing == sessionsByUserId_.end())
    {
        return false;
    }

    const auto now =
        std::chrono::steady_clock::now();

    if (existing->second.expiresAt <= now)
    {
        sessionsByUserId_.erase(existing);
        return false;
    }

    if (existing->second.sessionId != sessionId)
    {
        return false;
    }

    existing->second.expiresAt =
        now + sessionTtl_;

    return true;
}

void InMemorySessionStore::release(
    UserId userId,
    const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(
        mutex_
    );

    const auto existing =
        sessionsByUserId_.find(userId);

    if (existing == sessionsByUserId_.end())
    {
        return;
    }

    /*
     * רק הבעלים הנוכחי רשאי להסיר את הרשומה.
     */
    if (existing->second.sessionId != sessionId)
    {
        return;
    }

    sessionsByUserId_.erase(existing);
}