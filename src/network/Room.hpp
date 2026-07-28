#pragma once

#include "config/PieceSpeedConfig.hpp"
#include "controllerClick/GameController.hpp"
#include "game_engine/GameEngine.hpp"
#include "model/Board.hpp"
#include "network/GameLoop.hpp"
#include "network/MessageRouter.hpp"
#include "network/PlayerRole.hpp"

#include <boost/asio.hpp>

#include <memory>
#include <set>
#include <string>

class ClientSession;

class Room
{
public:
    using SessionPtr =
        std::shared_ptr<ClientSession>;

    Room(
        std::string id,
        boost::asio::io_context& ioContext,
        Board board,
        PieceSpeedConfig speedConfig,
        MessageRouter::LoginHandler loginHandler
    );

    void start();

    void stop();

    void addSession(
        const SessionPtr& session
    );

    void removeSession(
        const SessionPtr& session
    );

    void routeMessage(
        const SessionPtr& session,
        const std::string& message
    );

    [[nodiscard]]
    bool contains(
        const SessionPtr& session
    ) const;

    [[nodiscard]]
    const std::string& id() const;

private:
    void broadcast(
        const std::string& message
    );

    [[nodiscard]]
    PlayerRole assignRole() const;

    [[nodiscard]]
    bool isRoleOccupied(
        PlayerRole role
    ) const;

private:
    std::string id_;

    std::set<SessionPtr> sessions_;

    PieceSpeedConfig speedConfig_;

    GameEngine engine_;

    GameController controller_;

    MessageRouter messageRouter_;

    GameLoop gameLoop_;
};