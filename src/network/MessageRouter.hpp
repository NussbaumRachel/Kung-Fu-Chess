#pragma once

#include "network/LoginAttemptResult.hpp"
#include "network/Messages.hpp"

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

    MessageRouter(
        LoginHandler loginHandler,
        ClickHandler clickHandler
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

    void sendError(
        const SessionPtr& session,
        const std::string& errorMessage
    ) const;

private:
    LoginHandler loginHandler_;

    ClickHandler clickHandler_;
};