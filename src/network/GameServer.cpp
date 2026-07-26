#include "network/GameServer.hpp"

#include "network/ClientSession.hpp"
#include "network/JsonProtocol.hpp"
#include "network/Messages.hpp"

#include "controllerClick/GameController.hpp"
#include "game_engine/GameSnapshot.hpp"

#include <boost/asio.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

GameServer::GameServer(
    uint16_t port,
    GameController& controller)
    :
    ioContext_(),
    acceptor_(
        ioContext_,
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(),
            port
        )
    ),
    controller_(controller),
    tickTimer_(ioContext_)
{
    std::cout
        << "Server listening on port "
        << port
        << std::endl;
}

void GameServer::run()
{
    acceptNext();
    startGameLoop();
    ioContext_.run();
}

void GameServer::onSessionReady(
    std::shared_ptr<ClientSession> session)
{
    sessions_.insert(session);

    std::string assignedColor;

    if (nextColor_ == 0)
    {
        assignedColor = "White";
    }
    else if (nextColor_ == 1)
    {
        assignedColor = "Black";
    }
    else
    {
        assignedColor = "Spectator";
    }

    ++nextColor_;

    std::cout
        << "Player joined as "
        << assignedColor
        << std::endl;

    const WelcomeMessage welcome{
        assignedColor
    };

    session->send(
        JsonProtocol::serializeWelcome(welcome)
    );

    const GameSnapshot snapshot =
        controller_.getSnapshot();

    session->send(
        JsonProtocol::serializeSnapshot(snapshot)
    );
}

void GameServer::onMessage(
    std::shared_ptr<ClientSession> session,
    const std::string& message)
{
    std::cout
        << "SERVER RECEIVED: "
        << message
        << std::endl;

    try
    {
        const MessageType messageType =
            JsonProtocol::getMessageType(message);

        switch (messageType)
        {
            case MessageType::Click:
            {
                const ClickMessage click =
                    JsonProtocol::deserializeClick(message);

                if (click.row < 0 || click.col < 0)
                {
                    const ErrorMessage error{
                        "Click coordinates must be non-negative"
                    };

                    session->send(
                        JsonProtocol::serializeError(error)
                    );

                    return;
                }

                std::cout
                    << "Click: ("
                    << click.row
                    << ", "
                    << click.col
                    << ")"
                    << std::endl;

                controller_.handleCellClick(
                    click.row,
                    click.col
                );

                break;
            }

            case MessageType::Welcome:
            case MessageType::Snapshot:
            case MessageType::Error:
            {
                const ErrorMessage error{
                    "Message type is not accepted from clients"
                };

                session->send(
                    JsonProtocol::serializeError(error)
                );

                break;
            }

            case MessageType::Unknown:
            default:
            {
                const ErrorMessage error{
                    "Unknown message type"
                };

                session->send(
                    JsonProtocol::serializeError(error)
                );

                break;
            }
        }
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "Message parse error: "
            << exception.what()
            << std::endl;

        const ErrorMessage error{
            exception.what()
        };

        session->send(
            JsonProtocol::serializeError(error)
        );
    }
}

void GameServer::onSessionClosed(
    std::shared_ptr<ClientSession> session)
{
    sessions_.erase(session);

    std::cout
        << "Player disconnected"
        << std::endl;
}

void GameServer::startGameLoop()
{
    tickTimer_.expires_after(
        std::chrono::milliseconds(TICK_MS)
    );

    tickTimer_.async_wait(
        [this](boost::system::error_code error)
        {
            if (!error)
                tick();
        }
    );
}

void GameServer::tick()
{
    controller_.handleWait(TICK_MS);

    const GameSnapshot snapshot =
        controller_.getSnapshot();

    const std::string serializedSnapshot =
        JsonProtocol::serializeSnapshot(snapshot);

    for (const auto& session : sessions_)
    {
        session->send(serializedSnapshot);
    }

    tickTimer_.expires_after(
        std::chrono::milliseconds(TICK_MS)
    );

    tickTimer_.async_wait(
        [this](boost::system::error_code error)
        {
            if (!error)
                tick();
        }
    );
}

void GameServer::acceptNext()
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
                        *this
                    );

                session->start();
            }
            else
            {
                std::cout
                    << "Accept error: "
                    << error.message()
                    << std::endl;
            }

            acceptNext();
        }
    );
}