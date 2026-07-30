#include "infrastructure/postgres/PostgresUserRepository.hpp"

#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <string>

PostgresUserRepository::PostgresUserRepository(
    const std::string& connectionString)
    : connection_(connectionString)
{
    if (!connection_.is_open())
    {
        throw std::runtime_error(
            "Failed to open PostgreSQL connection"
        );
    }
}

User PostgresUserRepository::getOrCreateByUsername(
    const std::string& username)
{
    /*
     * pqxx::connection אינו מיועד לשימוש מקביל.
     * כרגע השרת מפעיל io_context ב-thread יחיד, אך
     * ה-mutex שומר את ה-Adapter בטוח גם אם זה ישתנה.
     */
    std::lock_guard<std::mutex> lock(
        connectionMutex_
    );

    pqxx::work transaction(connection_);

    const pqxx::result result =
        transaction.exec_params(
            R"SQL(
                INSERT INTO users (username)
                VALUES ($1)
                ON CONFLICT (username)
                DO UPDATE
                    SET username = EXCLUDED.username
                RETURNING id, username
            )SQL",
            username
        );

    if (result.size() != 1)
    {
        throw std::runtime_error(
            "PostgreSQL did not return the user row"
        );
    }

    const pqxx::row row = result[0];

    const User user(
        row["id"].as<std::int64_t>(),
        row["username"].as<std::string>()
    );

    transaction.commit();

    return user;
}