#include "network/GameServer.hpp"

#include "network/ClientSession.hpp"
#include "network/LoginAttemptResult.hpp"

#include <memory>
#include <string>
#include <utility>

GameServer::GameServer(
    std::uint16_t port,
    Board boardTemplate,
    PieceSpeedConfig speedConfig
)
    : ioContext_(),

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
              onSessionMessage(
                  std::move(session),
                  message
              );
          },

          [this](
              std::shared_ptr<ClientSession> session)
          {
              onSessionClosed(
                  std::move(session)
              );
          }
      ),

      roomManager_(
          ioContext_,
          std::move(boardTemplate),
          std::move(speedConfig),

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
      )
{
}

void GameServer::run()
{
    sessionManager_.start();
    roomManager_.start();

    ioContext_.run();
}

void GameServer::onSessionReady(
    std::shared_ptr<ClientSession> session)
{
    roomManager_.addToDefaultRoom(
        session
    );
}

void GameServer::onSessionMessage(
    std::shared_ptr<ClientSession> session,
    const std::string& message)
{
    roomManager_.routeMessage(
        session,
        message
    );
}

void GameServer::onSessionClosed(
    std::shared_ptr<ClientSession> session)
{
    roomManager_.removeSession(
        session
    );
}