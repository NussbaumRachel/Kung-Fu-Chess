#include "network/MessageRouter.hpp"

#include "network/ClientSession.hpp"
#include "network/JsonProtocol.hpp"

#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

MessageRouter::MessageRouter(
    LoginHandler loginHandler,
    ClickHandler clickHandler,
    CreateRoomHandler createRoomHandler,
    JoinRoomHandler joinRoomHandler,
    LeaveRoomHandler leaveRoomHandler
)
    : loginHandler_(
          std::move(loginHandler)
      ),
      clickHandler_(
          std::move(clickHandler)
      ),
      createRoomHandler_(
          std::move(createRoomHandler)
      ),
      joinRoomHandler_(
          std::move(joinRoomHandler)
      ),
      leaveRoomHandler_(
          std::move(leaveRoomHandler)
      )
{
}

void MessageRouter::route(
    SessionPtr session,
    const std::string& message)
{
    if (!session)
    {
        std::cerr
            << "MessageRouter received a null session"
            << std::endl;

        return;
    }

    try
    {
        const MessageType messageType =
            JsonProtocol::getMessageType(
                message
            );

        switch (messageType)
        {
            case MessageType::Login:
            {
                handleLogin(
                    session,
                    message
                );

                break;
            }

            case MessageType::Click:
            {
                handleClick(
                    session,
                    message
                );

                break;
            }

            case MessageType::CreateRoom:
            {
                handleCreateRoom(
                    session,
                    message
                );

                break;
            }

            case MessageType::JoinRoom:
            {
                handleJoinRoom(
                    session,
                    message
                );

                break;
            }

            case MessageType::LeaveRoom:
            {
                handleLeaveRoom(
                    session,
                    message
                );

                break;
            }

            case MessageType::Welcome:
            case MessageType::LoginResult:
            case MessageType::Snapshot:
            case MessageType::RoomResult:
            case MessageType::Error:
            {
                sendError(
                    session,
                    "Message type is not accepted from clients"
                );

                break;
            }

            case MessageType::Unknown:
            default:
            {
                sendError(
                    session,
                    "Unknown message type"
                );

                break;
            }
        }
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "Message processing error: "
            << exception.what()
            << std::endl;

        sendError(
            session,
            exception.what()
        );
    }
}

void MessageRouter::handleLogin(
    const SessionPtr& session,
    const std::string& message)
{
    const LoginMessage login =
        JsonProtocol::deserializeLogin(
            message
        );

    if (!loginHandler_)
    {
        sendError(
            session,
            "Login service is unavailable"
        );

        return;
    }

    const LoginAttemptResult result =
        loginHandler_(
            session,
            login.username
        );

    const LoginResultMessage response{
        result.success,
        result.success
            ? login.username
            : "",
        result.message
    };

    session->send(
        JsonProtocol::serializeLoginResult(
            response
        )
    );
}

void MessageRouter::handleClick(
    const SessionPtr& session,
    const std::string& message)
{
    if (!requireAuthentication(session))
    {
        return;
    }

    const ClickMessage click =
        JsonProtocol::deserializeClick(
            message
        );

    if (
        click.row < 0 ||
        click.col < 0
    )
    {
        sendError(
            session,
            "Click coordinates must be non-negative"
        );

        return;
    }

    if (!clickHandler_)
    {
        sendError(
            session,
            "Game action service is unavailable"
        );

        return;
    }

    clickHandler_(
        session,
        click
    );
}

void MessageRouter::handleCreateRoom(
    const SessionPtr& session,
    const std::string& message)
{
    if (!requireAuthentication(session))
    {
        return;
    }

    const CreateRoomMessage request =
        JsonProtocol::deserializeCreateRoom(
            message
        );

    if (!createRoomHandler_)
    {
        sendError(
            session,
            "Room creation service is unavailable"
        );

        return;
    }

    const RoomOperationResult result =
        createRoomHandler_(
            session,
            request.roomId
        );

    sendRoomResult(
        session,
        "create",
        request.roomId,
        result
    );
}

void MessageRouter::handleJoinRoom(
    const SessionPtr& session,
    const std::string& message)
{
    if (!requireAuthentication(session))
    {
        return;
    }

    const JoinRoomMessage request =
        JsonProtocol::deserializeJoinRoom(
            message
        );

    if (!joinRoomHandler_)
    {
        sendError(
            session,
            "Room joining service is unavailable"
        );

        return;
    }

    const RoomOperationResult result =
        joinRoomHandler_(
            session,
            request.roomId
        );

    sendRoomResult(
        session,
        "join",
        request.roomId,
        result
    );
}

void MessageRouter::handleLeaveRoom(
    const SessionPtr& session,
    const std::string& message)
{
    if (!requireAuthentication(session))
    {
        return;
    }

    static_cast<void>(
        JsonProtocol::deserializeLeaveRoom(
            message
        )
    );

    if (!leaveRoomHandler_)
    {
        sendError(
            session,
            "Room leaving service is unavailable"
        );

        return;
    }

    const RoomOperationResult result =
        leaveRoomHandler_(
            session
        );

    sendRoomResult(
        session,
        "leave",
        "",
        result
    );
}

bool MessageRouter::requireAuthentication(
    const SessionPtr& session) const
{
    if (
        session &&
        session->isAuthenticated()
    )
    {
        return true;
    }

    sendError(
        session,
        "Login is required before using this action"
    );

    return false;
}

void MessageRouter::sendRoomResult(
    const SessionPtr& session,
    const std::string& action,
    const std::string& roomId,
    const RoomOperationResult& result) const
{
    if (!session)
    {
        return;
    }

    const RoomResultMessage response{
        result.success,
        action,
        roomId,
        result.message
    };

    session->send(
        JsonProtocol::serializeRoomResult(
            response
        )
    );
}

void MessageRouter::sendError(
    const SessionPtr& session,
    const std::string& errorMessage) const
{
    if (!session)
    {
        return;
    }

    const ErrorMessage error{
        errorMessage
    };

    session->send(
        JsonProtocol::serializeError(
            error
        )
    );
}