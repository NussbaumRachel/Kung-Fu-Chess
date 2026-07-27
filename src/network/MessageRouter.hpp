#pragma once

#include "network/PlayerRole.hpp"

#include <memory>
#include <string>

class ClientSession;
class GameController;
struct GameSnapshot;
struct PieceInfo;

class MessageRouter
{
public:
    explicit MessageRouter(
        GameController& controller
    );

    void route(
        std::shared_ptr<ClientSession> session,
        const std::string& message
    );

private:
    void handleClick(
        const std::shared_ptr<ClientSession>& session,
        const std::string& message
    );

    [[nodiscard]]
    bool authorizeClick(
        const std::shared_ptr<ClientSession>& session,
        int row,
        int col,
        std::string& rejectionReason
    ) const;

    [[nodiscard]]
    const PieceInfo* findPieceAt(
        const GameSnapshot& snapshot,
        int row,
        int col
    ) const;

    void sendError(
        const std::shared_ptr<ClientSession>& session,
        const std::string& errorMessage
    );

private:
    GameController& controller_;
};