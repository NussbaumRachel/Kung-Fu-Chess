#include "network/GameServer.hpp"

#include "network/ClientSession.hpp"
#include "network/JsonProtocol.hpp"

#include "controllerClick/GameController.hpp"
#include "game_engine/GameSnapshot.hpp"

#include <memory>
#include <string>
#include <utility>

GameServer::GameServer(
    std::uint16_t port,
    GameController& controller
)
    : ioContext_(),
      messageRouter_(controller),
      sessionManager_(
          ioContext_,
          port,

          [this](
              std::shared_ptr<ClientSession> session)
          {
              onSessionReady(
                  std::move(session)
              );
          },

          [this](
              std::shared_ptr<ClientSession> session,
              const std::string& message)
          {
              messageRouter_.route(
                  std::move(session),
                  message
              );
          }
      ),
      gameLoop_(
          ioContext_,
          controller,

          [this](const std::string& message)
          {
              sessionManager_.broadcast(message);
          }
      ),
      controller_(controller)
{
}

void GameServer::run()
{
    sessionManager_.start();
    gameLoop_.start();

    ioContext_.run();
}

void GameServer::onSessionReady(
    std::shared_ptr<ClientSession> session)
{
    if (!session)
        return;

    const GameSnapshot snapshot =
        controller_.getSnapshot();

    session->send(
        JsonProtocol::serializeSnapshot(snapshot)
    );
}