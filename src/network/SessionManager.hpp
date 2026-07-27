#pragma once

#include "network/PlayerRole.hpp"

#include <boost/asio.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <string>

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

    SessionManager(
        boost::asio::io_context& ioContext,
        std::uint16_t port,
        ReadyHandler onReady,
        MessageHandler onMessage
    );

    void start();

    void broadcast(const std::string& message);

private:
    void acceptNext();

    void handleSessionReady(SessionPtr session);

    void handleSessionClosed(SessionPtr session);

    [[nodiscard]]
    PlayerRole assignRole();

private:
    boost::asio::ip::tcp::acceptor acceptor_;

    ReadyHandler onReady_;
    MessageHandler onMessage_;

    std::set<SessionPtr> sessions_;

    std::size_t nextRoleIndex_ = 0;
};