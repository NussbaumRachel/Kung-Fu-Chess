#include "infrastructure/redis/RedisSessionStore.hpp"

#include <hiredis/hiredis.h>

#include <chrono>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
    class RedisReplyGuard
    {
    public:
        explicit RedisReplyGuard(
            redisReply* reply)
            : reply_(reply)
        {
        }

        ~RedisReplyGuard()
        {
            if (reply_ != nullptr)
            {
                freeReplyObject(reply_);
            }
        }

        RedisReplyGuard(
            const RedisReplyGuard&
        ) = delete;

        RedisReplyGuard& operator=(
            const RedisReplyGuard&
        ) = delete;

        [[nodiscard]]
        redisReply* get() const
        {
            return reply_;
        }

    private:
        redisReply* reply_;
    };

    [[noreturn]]
    void throwRedisError(
        const std::string& message,
        const redisContext* context)
    {
        if (
            context != nullptr &&
            context->errstr != nullptr &&
            context->errstr[0] != '\0'
        )
        {
            throw std::runtime_error(
                message +
                ": " +
                context->errstr
            );
        }

        throw std::runtime_error(message);
    }
}

RedisSessionStore::RedisSessionStore(
    RedisConnectionConfig config)
    : config_(std::move(config))
{
    connect();
}

RedisSessionStore::~RedisSessionStore()
{
    std::lock_guard<std::mutex> lock(
        mutex_
    );

    disconnect();
}

bool RedisSessionStore::tryAcquire(
    UserId userId,
    const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(
        mutex_
    );

    if (context_ == nullptr)
    {
        connect();
    }

    const std::string key =
        makeSessionKey(userId);

    auto* rawReply =
        static_cast<redisReply*>(
            redisCommand(
                context_,
                "SET %b %b NX EX %d",
                key.data(),
                key.size(),
                sessionId.data(),
                sessionId.size(),
                config_.sessionTtlSeconds
            )
        );

    RedisReplyGuard reply(rawReply);

    if (reply.get() == nullptr)
    {
        throwRedisError(
            "Redis SET command failed",
            context_
        );
    }

    if (
        reply.get()->type ==
        REDIS_REPLY_STATUS
    )
    {
        const std::string result(
            reply.get()->str,
            reply.get()->len
        );

        return result == "OK";
    }

    /*
     * Redis מחזיר NIL כאשר NX נכשל,
     * כלומר למשתמש כבר קיים session.
     */
    if (
        reply.get()->type ==
        REDIS_REPLY_NIL
    )
    {
        return false;
    }

    if (
        reply.get()->type ==
        REDIS_REPLY_ERROR
    )
    {
        throw std::runtime_error(
            std::string(
                "Redis SET returned an error: "
            ) +
            std::string(
                reply.get()->str,
                reply.get()->len
            )
        );
    }

    throw std::runtime_error(
        "Redis SET returned an unexpected reply type"
    );
}
bool RedisSessionStore::refresh(
    UserId userId,
    const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(
        mutex_
    );

    if (context_ == nullptr)
    {
        connect();
    }

    const std::string key =
        makeSessionKey(userId);

    /*
     * הפעולה אטומית:
     * ה-TTL מתחדש רק אם sessionId עדיין
     * תואם לבעלים הנוכחי של המפתח.
     */
    static constexpr const char* refreshScript =
        "if redis.call('GET', KEYS[1]) == ARGV[1] "
        "then "
        "return redis.call('EXPIRE', KEYS[1], ARGV[2]) "
        "else "
        "return 0 "
        "end";

    const std::string ttlSeconds =
        std::to_string(
            config_.sessionTtlSeconds
        );

    auto* rawReply =
        static_cast<redisReply*>(
            redisCommand(
                context_,
                "EVAL %s 1 %b %b %b",
                refreshScript,
                key.data(),
                key.size(),
                sessionId.data(),
                sessionId.size(),
                ttlSeconds.data(),
                ttlSeconds.size()
            )
        );

    RedisReplyGuard reply(rawReply);

    if (reply.get() == nullptr)
    {
        throwRedisError(
            "Redis refresh command failed",
            context_
        );
    }

    if (
        reply.get()->type ==
        REDIS_REPLY_ERROR
    )
    {
        throw std::runtime_error(
            std::string(
                "Redis refresh returned an error: "
            ) +
            std::string(
                reply.get()->str,
                reply.get()->len
            )
        );
    }

    if (
        reply.get()->type !=
        REDIS_REPLY_INTEGER
    )
    {
        throw std::runtime_error(
            "Redis refresh returned an unexpected reply type"
        );
    }

    return reply.get()->integer == 1;
}
void RedisSessionStore::release(
    UserId userId,
    const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(
        mutex_
    );

    if (context_ == nullptr)
    {
        connect();
    }

    const std::string key =
        makeSessionKey(userId);

    /*
     * הפעולה אטומית:
     * המפתח נמחק רק אם sessionId עדיין
     * תואם לבעלים הנוכחי.
     */
    static constexpr const char* releaseScript =
        "if redis.call('GET', KEYS[1]) == ARGV[1] "
        "then "
        "return redis.call('DEL', KEYS[1]) "
        "else "
        "return 0 "
        "end";

    auto* rawReply =
        static_cast<redisReply*>(
            redisCommand(
                context_,
                "EVAL %s 1 %b %b",
                releaseScript,
                key.data(),
                key.size(),
                sessionId.data(),
                sessionId.size()
            )
        );

    RedisReplyGuard reply(rawReply);

    if (reply.get() == nullptr)
    {
        throwRedisError(
            "Redis release command failed",
            context_
        );
    }

    if (
        reply.get()->type ==
        REDIS_REPLY_ERROR
    )
    {
        throw std::runtime_error(
            std::string(
                "Redis release returned an error: "
            ) +
            std::string(
                reply.get()->str,
                reply.get()->len
            )
        );
    }

    if (
        reply.get()->type !=
        REDIS_REPLY_INTEGER
    )
    {
        throw std::runtime_error(
            "Redis release returned an unexpected reply type"
        );
    }
}

void RedisSessionStore::connect()
{
    disconnect();

    timeval timeout{};
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    context_ =
        redisConnectWithTimeout(
            config_.host.c_str(),
            config_.port,
            timeout
        );

    if (context_ == nullptr)
    {
        throw std::runtime_error(
            "Could not allocate Redis connection"
        );
    }

    if (context_->err != 0)
    {
        const std::string errorMessage =
            context_->errstr;

        disconnect();

        throw std::runtime_error(
            "Could not connect to Redis: " +
            errorMessage
        );
    }

    authenticate();
}

void RedisSessionStore::disconnect() noexcept
{
    if (context_ != nullptr)
    {
        redisFree(context_);
        context_ = nullptr;
    }
}

void RedisSessionStore::authenticate()
{
    if (config_.password.empty())
    {
        return;
    }

    auto* rawReply =
        static_cast<redisReply*>(
            redisCommand(
                context_,
                "AUTH %b",
                config_.password.data(),
                config_.password.size()
            )
        );

    RedisReplyGuard reply(rawReply);

    if (reply.get() == nullptr)
    {
        throwRedisError(
            "Redis authentication failed",
            context_
        );
    }

    if (
        reply.get()->type ==
        REDIS_REPLY_ERROR
    )
    {
        throw std::runtime_error(
            std::string(
                "Redis authentication failed: "
            ) +
            std::string(
                reply.get()->str,
                reply.get()->len
            )
        );
    }

    if (
        reply.get()->type !=
        REDIS_REPLY_STATUS
    )
    {
        throw std::runtime_error(
            "Redis authentication returned an unexpected reply type"
        );
    }

    const std::string result(
        reply.get()->str,
        reply.get()->len
    );

    if (result != "OK")
    {
        throw std::runtime_error(
            "Redis authentication was not accepted"
        );
    }
}

std::string RedisSessionStore::makeSessionKey(
    UserId userId)
{
    std::ostringstream stream;

    stream
        << "kung-fu-chess:session:user:"
        << userId;

    return stream.str();
}