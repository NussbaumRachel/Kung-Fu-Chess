#pragma once

#include <boost/asio.hpp>

#include <functional>
#include <string>

class GameController;

class GameLoop
{
public:
    using BroadcastHandler =
        std::function<void(const std::string&)>;

    GameLoop(
        boost::asio::io_context& ioContext,
        GameController& controller,
        BroadcastHandler broadcastHandler
    );

    void start();

    void stop();

private:
    void scheduleNextTick();

    void tick();

private:
    static constexpr int TICK_MS = 16;

    GameController& controller_;

    BroadcastHandler broadcastHandler_;

    boost::asio::steady_timer timer_;

    bool running_ = false;
};