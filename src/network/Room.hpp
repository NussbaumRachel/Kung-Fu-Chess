#pragma once

#include "config/PieceSpeedConfig.hpp"
#include "controllerClick/GameController.hpp"
#include "game_engine/GameEngine.hpp"
#include "game_engine/GameSnapshot.hpp"
#include "model/Board.hpp"
#include "network/GameLoop.hpp"
#include "network/Messages.hpp"
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
        PieceSpeedConfig speedConfig
    );

    void start();

    void stop();

    void addSession(
        const SessionPtr& session
    );

    void removeSession(
        const SessionPtr& session
    );

    void handleClick(
        const SessionPtr& session,
        const ClickMessage& click
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

    void sendError(
        const SessionPtr& session,
        const std::string& errorMessage
    ) const;

    [[nodiscard]]
    PlayerRole assignRole() const;

    [[nodiscard]]
    bool isRoleOccupied(
        PlayerRole role
    ) const;

    [[nodiscard]]
    bool authorizeClick(
        const SessionPtr& session,
        int row,
        int col,
        const GameSnapshot& snapshot,
        std::string& rejectionReason
    );

    [[nodiscard]]
    const PieceInfo* findPieceAt(
        const GameSnapshot& snapshot,
        int row,
        int col
    ) const;

    void updateSelectionOwner(
        const SessionPtr& session,
        const GameSnapshot& snapshotBefore,
        const GameSnapshot& snapshotAfter
    );

    void clearExpiredSelectionOwner(
        const GameSnapshot& snapshot
    );

private:
    std::string id_;

    std::set<SessionPtr> sessions_;

    /*
     * סדר השדות חשוב:
     * GameEngine משתמש ב-PieceSpeedConfig.
     */
    PieceSpeedConfig speedConfig_;

    GameEngine engine_;

    GameController controller_;

    GameLoop gameLoop_;

    /*
     * הבחירה שייכת למשחק המסוים בחדר,
     * ולכן המצב נשמר כאן ולא ב-MessageRouter הגלובלי.
     */
    std::weak_ptr<ClientSession>
        selectionOwner_;
};