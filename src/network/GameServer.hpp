#pragma once

#include "network/GameLoop.hpp"
#include "network/MessageRouter.hpp"
#include "network/SessionManager.hpp"

#include <boost/asio.hpp>

#include <cstdint>
#include <memory>

class ClientSession;
class GameController;

class GameServer
{
public:
    GameServer(
        std::uint16_t port,
        GameController& controller
    );

    void run();

private:
    void onSessionReady(
        std::shared_ptr<ClientSession> session
    );

private:
    boost::asio::io_context ioContext_;

    MessageRouter messageRouter_;

    SessionManager sessionManager_;

    GameLoop gameLoop_;

    GameController& controller_;
};