#pragma once

#include "user/IUserRepository.hpp"

#include <pqxx/pqxx>

#include <mutex>
#include <string>

class PostgresUserRepository final
    : public IUserRepository
{
public:
    explicit PostgresUserRepository(
        const std::string& connectionString
    );

    PostgresUserRepository(
        const PostgresUserRepository&
    ) = delete;

    PostgresUserRepository& operator=(
        const PostgresUserRepository&
    ) = delete;

    [[nodiscard]]
    User getOrCreateByUsername(
        const std::string& username
    ) override;

private:
    pqxx::connection connection_;
    std::mutex connectionMutex_;
};