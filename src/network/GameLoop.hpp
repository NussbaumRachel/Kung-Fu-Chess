#pragma once

#include "game_engine/GameSnapshot.hpp"

#include <boost/asio.hpp>

#include <functional>
#include <string>

class GameController;

class GameLoop
{
public:
    using BroadcastHandler =
        std::function<void(const std::string&)>;

    using GameOverHandler =
        std::function<void(
            const GameSnapshot&
        )>;

    GameLoop(
        boost::asio::io_context& ioContext,
        GameController& controller,
        BroadcastHandler broadcastHandler,
        GameOverHandler gameOverHandler
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

    GameOverHandler gameOverHandler_;

    boost::asio::steady_timer timer_;

    bool running_ = false;

    bool gameOverHandled_ = false;
};