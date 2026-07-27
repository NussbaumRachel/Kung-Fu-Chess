#include "network/SessionManager.hpp"

#include "network/ClientSession.hpp"
#include "network/JsonProtocol.hpp"
#include "network/Messages.hpp"
#include "network/PlayerRole.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <utility>

SessionManager::SessionManager(
    boost::asio::io_context& ioContext,
    std::uint16_t port,
    ReadyHandler onReady,
    MessageHandler onMessage
)
    : acceptor_(
          ioContext,
          boost::asio::ip::tcp::endpoint(
              boost::asio::ip::tcp::v4(),
              port
          )
      ),
      onReady_(std::move(onReady)),
      onMessage_(std::move(onMessage))
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

void SessionManager::broadcast(
    const std::string& message)
{
    for (const SessionPtr& session : sessions_)
    {
        session->send(message);
    }
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
    const PlayerRole assignedRole =
        assignRole();

    session->setRole(assignedRole);

    sessions_.insert(session);

    const std::string roleName{
        playerRoleToString(assignedRole)
    };

    std::cout
        << "Client joined as "
        << roleName
        << std::endl;

    const WelcomeMessage welcome{
        roleName
    };

    session->send(
        JsonProtocol::serializeWelcome(welcome)
    );

    if (onReady_)
        onReady_(std::move(session));
}

void SessionManager::handleSessionClosed(
    SessionPtr session)
{
    sessions_.erase(session);

    std::cout
        << "Client disconnected"
        << std::endl;
}

PlayerRole SessionManager::assignRole()
{
    if (nextRoleIndex_ == 0)
    {
        ++nextRoleIndex_;
        return PlayerRole::White;
    }

    if (nextRoleIndex_ == 1)
    {
        ++nextRoleIndex_;
        return PlayerRole::Black;
    }

    ++nextRoleIndex_;
    return PlayerRole::Spectator;
}