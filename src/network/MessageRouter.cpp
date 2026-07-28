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
    ClickHandler clickHandler
)
    : loginHandler_(
          std::move(loginHandler)
      ),
      clickHandler_(
          std::move(clickHandler)
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

            case MessageType::Welcome:
            case MessageType::LoginResult:
            case MessageType::Snapshot:
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
    if (!session->isAuthenticated())
    {
        sendError(
            session,
            "Login is required before playing"
        );

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