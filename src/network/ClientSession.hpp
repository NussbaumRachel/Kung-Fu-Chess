#pragma once

#include "network/PlayerRole.hpp"
#include "user/User.hpp"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>

class ClientSession
    : public std::enable_shared_from_this<ClientSession>
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

    using RefreshHandler =
        std::function<bool(SessionPtr)>;

    using ClosedHandler =
        std::function<void(SessionPtr)>;

    ClientSession(
        boost::asio::ip::tcp::socket socket,
        ReadyHandler onReady,
        MessageHandler onMessage,
        RefreshHandler onRefresh,
        ClosedHandler onClosed
    );

    void start();

    void send(const std::string& message);

    void setRole(PlayerRole role);

    [[nodiscard]]
    PlayerRole role() const;

    void authenticate(
        UserId userId,
        std::string username
    );

    void clearAuthentication();

    [[nodiscard]]
    bool isAuthenticated() const;

    [[nodiscard]]
    std::optional<UserId> userId() const;

    [[nodiscard]]
    const std::string& username() const;

    [[nodiscard]]
    const std::string& sessionId() const;

private:
    static std::string generateSessionId();

    void doHandshake();

    void read();

    void writeNext();

    void startSessionRefresh();

    void cancelSessionRefresh();

    void notifyClosed();

private:
    boost::beast::websocket::stream<
        boost::asio::ip::tcp::socket
    > ws_;

    boost::beast::flat_buffer readBuffer_;

    boost::asio::steady_timer
        sessionRefreshTimer_;

    ReadyHandler onReady_;
    MessageHandler onMessage_;
    RefreshHandler onRefresh_;
    ClosedHandler onClosed_;

    std::mutex writeMutex_;

    std::queue<
        std::shared_ptr<std::string>
    > writeQueue_;

    bool writing_ = false;
    bool closedNotified_ = false;
    bool refreshStarted_ = false;

    PlayerRole role_ =
        PlayerRole::Spectator;

    const std::string sessionId_;

    std::optional<UserId> userId_;
    std::string username_;
};