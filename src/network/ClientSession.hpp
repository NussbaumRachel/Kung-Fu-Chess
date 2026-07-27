#pragma once

#include "network/PlayerRole.hpp"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

class ClientSession
    : public std::enable_shared_from_this<ClientSession>
{
public:
    using SessionPtr = std::shared_ptr<ClientSession>;

    using ReadyHandler =
        std::function<void(SessionPtr)>;

    using MessageHandler =
        std::function<void(
            SessionPtr,
            const std::string&
        )>;

    using ClosedHandler =
        std::function<void(SessionPtr)>;

    ClientSession(
        boost::asio::ip::tcp::socket socket,
        ReadyHandler onReady,
        MessageHandler onMessage,
        ClosedHandler onClosed
    );

    void start();

    void send(const std::string& message);

    void setRole(PlayerRole role);

    [[nodiscard]]
    PlayerRole role() const;

private:
    void doHandshake();

    void read();

    void writeNext();

    void notifyClosed();

private:
    boost::beast::websocket::stream<
        boost::asio::ip::tcp::socket
    > ws_;

    boost::beast::flat_buffer readBuffer_;

    ReadyHandler onReady_;
    MessageHandler onMessage_;
    ClosedHandler onClosed_;

    std::mutex writeMutex_;

    std::queue<
        std::shared_ptr<std::string>
    > writeQueue_;

    bool writing_ = false;
    bool closedNotified_ = false;

    PlayerRole role_ = PlayerRole::Spectator;
};