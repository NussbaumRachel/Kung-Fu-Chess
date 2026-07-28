#include "network/Room.hpp"

#include "game_engine/GameSnapshot.hpp"
#include "network/ClientSession.hpp"
#include "network/JsonProtocol.hpp"
#include "network/Messages.hpp"
#include "network/PlayerRole.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <utility>

Room::Room(
    std::string id,
    boost::asio::io_context& ioContext,
    Board board,
    PieceSpeedConfig speedConfig,
    MessageRouter::LoginHandler loginHandler
)
    : id_(std::move(id)),
      speedConfig_(std::move(speedConfig)),
      engine_(
          std::move(board),
          speedConfig_
      ),
      controller_(engine_),
      messageRouter_(
          controller_,
          std::move(loginHandler)
      ),
      gameLoop_(
          ioContext,
          controller_,
          [this](const std::string& message)
          {
              broadcast(message);
          }
      )
{
}

void Room::start()
{
    gameLoop_.start();
}

void Room::stop()
{
    gameLoop_.stop();
}

void Room::addSession(
    const SessionPtr& session)
{
    if (!session)
        return;

    if (contains(session))
        return;

    const PlayerRole role =
        assignRole();

    session->setRole(role);
    sessions_.insert(session);

    const std::string roleName{
        playerRoleToString(role)
    };

    std::cout
        << "Client joined room '"
        << id_
        << "' as "
        << roleName
        << std::endl;

    const WelcomeMessage welcome{
        roleName
    };

    session->send(
        JsonProtocol::serializeWelcome(
            welcome
        )
    );

    const GameSnapshot snapshot =
        controller_.getSnapshot();

    session->send(
        JsonProtocol::serializeSnapshot(
            snapshot
        )
    );
}

void Room::removeSession(
    const SessionPtr& session)
{
    if (!session)
        return;

    const std::size_t removed =
        sessions_.erase(session);

    if (removed == 0)
        return;

    session->setRole(
        PlayerRole::Spectator
    );

    std::cout
        << "Client left room '"
        << id_
        << "'"
        << std::endl;
}

void Room::routeMessage(
    const SessionPtr& session,
    const std::string& message)
{
    if (!session)
        return;

    if (!contains(session))
        return;

    messageRouter_.route(
        session,
        message
    );
}

bool Room::contains(
    const SessionPtr& session) const
{
    if (!session)
        return false;

    return sessions_.find(session) !=
           sessions_.end();
}

const std::string& Room::id() const
{
    return id_;
}

void Room::broadcast(
    const std::string& message)
{
    for (const SessionPtr& session :
         sessions_)
    {
        if (session)
            session->send(message);
    }
}

PlayerRole Room::assignRole() const
{
    if (!isRoleOccupied(PlayerRole::White))
        return PlayerRole::White;

    if (!isRoleOccupied(PlayerRole::Black))
        return PlayerRole::Black;

    return PlayerRole::Spectator;
}

bool Room::isRoleOccupied(
    PlayerRole role) const
{
    for (const SessionPtr& session :
         sessions_)
    {
        if (
            session &&
            session->role() == role
        )
        {
            return true;
        }
    }

    return false;
}