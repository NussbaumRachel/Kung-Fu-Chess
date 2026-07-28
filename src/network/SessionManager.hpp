#pragma once

#include "network/LoginAttemptResult.hpp"

#include <boost/asio.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

class ClientSession;

class SessionManager
{
public:
    using SessionPtr =
        std::shared_ptr<ClientSession>;

    using ReadyHandler =
        std::function<void(SessionPtr)>;

    using MessageHandler =
        std::function<void(
            SessionPtr,
            const std::string&
        )>;

    using ClosedHandler =
        std::function<void(SessionPtr)>;

    SessionManager(
        boost::asio::io_context& ioContext,
        std::uint16_t port,
        ReadyHandler onReady,
        MessageHandler onMessage,
        ClosedHandler onClosed
    );

    void start();

    [[nodiscard]]
    LoginAttemptResult tryLogin(
        const SessionPtr& session,
        const std::string& username
    );

private:
    void acceptNext();

    void handleSessionReady(
        SessionPtr session
    );

    void handleSessionClosed(
        SessionPtr session
    );

    void releaseUsername(
        const SessionPtr& session
    );

    [[nodiscard]]
    static bool isValidUsername(
        const std::string& username
    );

private:
    boost::asio::ip::tcp::acceptor acceptor_;

    ReadyHandler onReady_;
    MessageHandler onMessage_;
    ClosedHandler onClosed_;

    std::set<SessionPtr> sessions_;

    std::unordered_map<
        std::string,
        std::weak_ptr<ClientSession>
    > usersByName_;
};