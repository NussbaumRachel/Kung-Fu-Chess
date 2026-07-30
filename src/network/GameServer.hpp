#pragma once

#include "config/PieceSpeedConfig.hpp"
#include "model/Board.hpp"
#include "network/MessageRouter.hpp"
#include "network/RoomManager.hpp"
#include "network/SessionManager.hpp"

#include <boost/asio.hpp>

#include <cstdint>
#include <memory>
#include <string>
class IGameResultRepository;
class ClientSession;
class ISessionStore;
class IUserRepository;

class GameServer
{
public:
    GameServer(
        std::uint16_t port,
        Board boardTemplate,
        PieceSpeedConfig speedConfig,
        IUserRepository& userRepository,
        ISessionStore& sessionStore,
        IGameResultRepository& gameResultRepository
    );
    void run();

private:
    void onSessionReady(
        std::shared_ptr<ClientSession> session
    );

    void onSessionMessage(
        std::shared_ptr<ClientSession> session,
        const std::string& message
    );

    void onSessionClosed(
        std::shared_ptr<ClientSession> session
    );

private:
    boost::asio::io_context ioContext_;

    SessionManager sessionManager_;

    RoomManager roomManager_;

    MessageRouter messageRouter_;
};