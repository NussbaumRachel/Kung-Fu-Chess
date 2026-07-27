#include "network/GameServer.hpp"

#include "network/ClientSession.hpp"
#include "network/JsonProtocol.hpp"
#include "network/LoginAttemptResult.hpp"

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
      controller_(controller),

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

      messageRouter_(
          controller_,

          [this](
              const std::shared_ptr<ClientSession>& session,
              const std::string& username)
              -> LoginAttemptResult
          {
              return sessionManager_.tryLogin(
                  session,
                  username
              );
          }
      ),

      gameLoop_(
          ioContext_,
          controller_,

          [this](const std::string& message)
          {
              sessionManager_.broadcast(
                  message
              );
          }
      )
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
        JsonProtocol::serializeSnapshot(
            snapshot
        )
    );
}