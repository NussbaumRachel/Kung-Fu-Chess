#pragma once

#include "network/LoginAttemptResult.hpp"
#include "network/Messages.hpp"
#include "network/RoomOperationResult.hpp"

#include <functional>
#include <memory>
#include <string>

class ClientSession;

class MessageRouter
{
public:
    using SessionPtr =
        std::shared_ptr<ClientSession>;

    using LoginHandler =
        std::function<LoginAttemptResult(
            const SessionPtr&,
            const std::string&
        )>;

    using ClickHandler =
        std::function<void(
            const SessionPtr&,
            const ClickMessage&
        )>;

    using CreateRoomHandler =
        std::function<RoomOperationResult(
            const SessionPtr&,
            const std::string&
        )>;

    using JoinRoomHandler =
        std::function<RoomOperationResult(
            const SessionPtr&,
            const std::string&
        )>;

    using LeaveRoomHandler =
        std::function<RoomOperationResult(
            const SessionPtr&
        )>;

    MessageRouter(
        LoginHandler loginHandler,
        ClickHandler clickHandler,
        CreateRoomHandler createRoomHandler,
        JoinRoomHandler joinRoomHandler,
        LeaveRoomHandler leaveRoomHandler
    );

    void route(
        SessionPtr session,
        const std::string& message
    );

private:
    void handleLogin(
        const SessionPtr& session,
        const std::string& message
    );

    void handleClick(
        const SessionPtr& session,
        const std::string& message
    );

    void handleCreateRoom(
        const SessionPtr& session,
        const std::string& message
    );

    void handleJoinRoom(
        const SessionPtr& session,
        const std::string& message
    );

    void handleLeaveRoom(
        const SessionPtr& session,
        const std::string& message
    );

    [[nodiscard]]
    bool requireAuthentication(
        const SessionPtr& session
    ) const;

    void sendRoomResult(
        const SessionPtr& session,
        const std::string& action,
        const std::string& roomId,
        const RoomOperationResult& result
    ) const;

    void sendError(
        const SessionPtr& session,
        const std::string& errorMessage
    ) const;

private:
    LoginHandler loginHandler_;

    ClickHandler clickHandler_;

    CreateRoomHandler createRoomHandler_;

    JoinRoomHandler joinRoomHandler_;

    LeaveRoomHandler leaveRoomHandler_;
};