#include "network/GameLoop.hpp"

#include "network/JsonProtocol.hpp"

#include "controllerClick/GameController.hpp"
#include "game_engine/GameSnapshot.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <utility>

GameLoop::GameLoop(
    boost::asio::io_context& ioContext,
    GameController& controller,
    BroadcastHandler broadcastHandler
)
    : controller_(controller),
      broadcastHandler_(
          std::move(broadcastHandler)
      ),
      timer_(ioContext)
{
}

void GameLoop::start()
{
    if (running_)
        return;

    running_ = true;

    scheduleNextTick();
}

void GameLoop::stop()
{
    if (!running_)
        return;

    running_ = false;

    timer_.cancel();
}

void GameLoop::scheduleNextTick()
{
    if (!running_)
        return;

    timer_.expires_after(
        std::chrono::milliseconds(TICK_MS)
    );

    timer_.async_wait(
        [this](
            const boost::system::error_code& error)
        {
            if (
                error ==
                boost::asio::error::operation_aborted
            )
            {
                return;
            }

            if (error)
            {
                std::cerr
                    << "Game loop timer error: "
                    << error.message()
                    << std::endl;

                running_ = false;
                return;
            }

            tick();
        }
    );
}

void GameLoop::tick()
{
    if (!running_)
        return;

    controller_.handleWait(TICK_MS);

    const GameSnapshot snapshot =
        controller_.getSnapshot();

    const std::string serializedSnapshot =
        JsonProtocol::serializeSnapshot(snapshot);

    if (broadcastHandler_)
    {
        broadcastHandler_(
            serializedSnapshot
        );
    }

    scheduleNextTick();
}