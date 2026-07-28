#include "network/SessionManager.hpp"

#include "network/ClientSession.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

SessionManager::SessionManager(
    boost::asio::io_context& ioContext,
    std::uint16_t port,
    ReadyHandler onReady,
    MessageHandler onMessage,
    ClosedHandler onClosed
)
    : acceptor_(
          ioContext,
          boost::asio::ip::tcp::endpoint(
              boost::asio::ip::tcp::v4(),
              port
          )
      ),
      onReady_(std::move(onReady)),
      onMessage_(std::move(onMessage)),
      onClosed_(std::move(onClosed))
{
    std::cout
        << "Server listening on port "
        << port
        << std::endl;
}

void SessionManager::start()
{
    acceptNext();
}

LoginAttemptResult SessionManager::tryLogin(
    const SessionPtr& session,
    const std::string& username)
{
    if (!session)
    {
        return {
            false,
            "Invalid client session"
        };
    }

    if (session->isAuthenticated())
    {
        return {
            false,
            "This client is already logged in"
        };
    }

    if (!isValidUsername(username))
    {
        return {
            false,
            "Username must contain 3 to 20 letters, digits or underscores"
        };
    }

    const auto existing =
        usersByName_.find(username);

    if (existing != usersByName_.end())
    {
        const std::shared_ptr<ClientSession>
            existingSession =
                existing->second.lock();

        if (existingSession)
        {
            return {
                false,
                "Username is already in use"
            };
        }

        usersByName_.erase(existing);
    }

    session->authenticate(username);
    usersByName_[username] = session;

    std::cout
        << "User logged in: "
        << username
        << std::endl;

    return {
        true,
        "Login successful"
    };
}

void SessionManager::acceptNext()
{
    acceptor_.async_accept(
        [this](
            boost::system::error_code error,
            boost::asio::ip::tcp::socket socket)
        {
            if (!error)
            {
                std::cout
                    << "New TCP connection"
                    << std::endl;

                auto session =
                    std::make_shared<ClientSession>(
                        std::move(socket),

                        [this](SessionPtr readySession)
                        {
                            handleSessionReady(
                                std::move(readySession)
                            );
                        },

                        [this](
                            SessionPtr messageSession,
                            const std::string& message)
                        {
                            if (onMessage_)
                            {
                                onMessage_(
                                    std::move(
                                        messageSession
                                    ),
                                    message
                                );
                            }
                        },

                        [this](SessionPtr closedSession)
                        {
                            handleSessionClosed(
                                std::move(closedSession)
                            );
                        }
                    );

                session->start();
            }
            else
            {
                std::cerr
                    << "Accept error: "
                    << error.message()
                    << std::endl;
            }

            acceptNext();
        }
    );
}

void SessionManager::handleSessionReady(
    SessionPtr session)
{
    if (!session)
        return;

    sessions_.insert(session);

    if (onReady_)
        onReady_(std::move(session));
}

void SessionManager::handleSessionClosed(
    SessionPtr session)
{
    if (!session)
        return;

    releaseUsername(session);

    if (onClosed_)
        onClosed_(session);

    sessions_.erase(session);

    std::cout
        << "Client disconnected"
        << std::endl;
}

void SessionManager::releaseUsername(
    const SessionPtr& session)
{
    if (
        !session ||
        !session->isAuthenticated()
    )
    {
        return;
    }

    const std::string username =
        session->username();

    const auto found =
        usersByName_.find(username);

    if (found != usersByName_.end())
    {
        const std::shared_ptr<ClientSession>
            registeredSession =
                found->second.lock();

        if (
            !registeredSession ||
            registeredSession.get() ==
                session.get()
        )
        {
            usersByName_.erase(found);
        }
    }

    session->clearAuthentication();

    std::cout
        << "Username released: "
        << username
        << std::endl;
}

bool SessionManager::isValidUsername(
    const std::string& username)
{
    if (
        username.size() < 3 ||
        username.size() > 20
    )
    {
        return false;
    }

    return std::all_of(
        username.begin(),
        username.end(),
        [](unsigned char character)
        {
            return
                std::isalnum(character) != 0 ||
                character == '_';
        }
    );
}