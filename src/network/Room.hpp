#pragma once

#include "config/PieceSpeedConfig.hpp"
#include "controllerClick/GameController.hpp"
#include "game_engine/GameEngine.hpp"
#include "game_engine/GameSnapshot.hpp"
#include "model/Board.hpp"
#include "network/GameLoop.hpp"
#include "network/Messages.hpp"
#include "network/PlayerRole.hpp"
#include "game_result/IGameResultRepository.hpp"
#include <boost/asio.hpp>
#include <optional>
#include <cstddef>
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
        IGameResultRepository& gameResultRepository
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
    bool empty() const;

    [[nodiscard]]
    std::size_t sessionCount() const;

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
    void handleGameOver(
    const GameSnapshot& snapshot
    );

    [[nodiscard]]
    std::optional<UserId> findUserIdByRole(
        PlayerRole role
    ) const;
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
     IGameResultRepository&
    gameResultRepository_;

bool gameResultSaveAttempted_ = false;
    std::weak_ptr<ClientSession>
        selectionOwner_;
};