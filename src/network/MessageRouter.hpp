#pragma once
#include "network/LoginAttemptResult.hpp"
#include <functional>
#include <memory>
#include <string>

class ClientSession;
class GameController;

struct GameSnapshot;
struct PieceInfo;

class MessageRouter
{
public:
    using LoginHandler =
        std::function<LoginAttemptResult(
            const std::shared_ptr<ClientSession>&,
            const std::string&
        )>;

    MessageRouter(
        GameController& controller,
        LoginHandler loginHandler
    );

    void route(
        std::shared_ptr<ClientSession> session,
        const std::string& message
    );

private:
    void handleLogin(
        const std::shared_ptr<ClientSession>& session,
        const std::string& message
    );

    void handleClick(
        const std::shared_ptr<ClientSession>& session,
        const std::string& message
    );

    [[nodiscard]]
    bool authorizeClick(
        const std::shared_ptr<ClientSession>& session,
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
        const std::shared_ptr<ClientSession>& session,
        const GameSnapshot& snapshotBefore,
        const GameSnapshot& snapshotAfter
    );

    void clearExpiredSelectionOwner(
        const GameSnapshot& snapshot
    );

    void sendError(
        const std::shared_ptr<ClientSession>& session,
        const std::string& errorMessage
    );

private:
    GameController& controller_;
    LoginHandler loginHandler_;

    std::weak_ptr<ClientSession>
        selectionOwner_;
};