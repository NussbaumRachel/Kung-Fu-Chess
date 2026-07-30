#include "network/SessionManager.hpp"

#include "network/ClientSession.hpp"
#include "session/ISessionStore.hpp"
#include "user/IUserRepository.hpp"
#include "user/User.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

SessionManager::SessionManager(
    boost::asio::io_context& ioContext,
    std::uint16_t port,
    IUserRepository& userRepository,
    ISessionStore& sessionStore,
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
      userRepository_(userRepository),
      sessionStore_(sessionStore),
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

    const User user =
        userRepository_.getOrCreateByUsername(
            username
        );

    const bool acquired =
        sessionStore_.tryAcquire(
            user.id,
            session->sessionId()
        );

    if (!acquired)
    {
        return {
            false,
            "Username is already in use"
        };
    }

    session->authenticate(
        user.id,
        user.username
    );

    std::cout
        << "User logged in: "
        << user.username
        << " (user_id="
        << user.id
        << ", session_id="
        << session->sessionId()
        << ")"
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

        [this](SessionPtr refreshSession)
        {
            return refreshAuthentication(
                refreshSession
            );
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
    {
        return;
    }

    sessions_.insert(session);

    if (onReady_)
    {
        onReady_(std::move(session));
    }
}

void SessionManager::handleSessionClosed(
    SessionPtr session)
{
    if (!session)
    {
        return;
    }

    releaseAuthentication(session);

    if (onClosed_)
    {
        onClosed_(session);
    }

    sessions_.erase(session);

    std::cout
        << "Client disconnected"
        << std::endl;
}
bool SessionManager::refreshAuthentication(
    const SessionPtr& session)
{
    if (
        !session ||
        !session->isAuthenticated()
    )
    {
        return false;
    }

    const std::optional<UserId> userId =
        session->userId();

    if (!userId.has_value())
    {
        return false;
    }

    const bool refreshed =
        sessionStore_.refresh(
            *userId,
            session->sessionId()
        );

    if (!refreshed)
    {
        std::cerr
            << "Failed to refresh session for user: "
            << session->username()
            << " (user_id="
            << *userId
            << ", session_id="
            << session->sessionId()
            << ")"
            << std::endl;
    }

    return refreshed;
}
void SessionManager::releaseAuthentication(
    const SessionPtr& session)
{
    if (
        !session ||
        !session->isAuthenticated()
    )
    {
        return;
    }

    const std::optional<UserId> userId =
        session->userId();

    const std::string username =
        session->username();

    if (userId.has_value())
    {
        sessionStore_.release(
            *userId,
            session->sessionId()
        );
    }

    session->clearAuthentication();

    std::cout
        << "Session released for user: "
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