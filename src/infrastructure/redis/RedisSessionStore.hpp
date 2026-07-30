#pragma once

#include "infrastructure/redis/RedisConnectionConfig.hpp"
#include "session/ISessionStore.hpp"

#include <mutex>
#include <string>

struct redisContext;

class RedisSessionStore final
    : public ISessionStore
{
public:
    explicit RedisSessionStore(
        RedisConnectionConfig config
    );

    ~RedisSessionStore() override;

    RedisSessionStore(
        const RedisSessionStore&
    ) = delete;

    RedisSessionStore& operator=(
        const RedisSessionStore&
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
    void connect();

    void disconnect() noexcept;

    void authenticate();

    [[nodiscard]]
    static std::string makeSessionKey(
        UserId userId
    );

private:
    RedisConnectionConfig config_;

    redisContext* context_ = nullptr;

    std::mutex mutex_;
};