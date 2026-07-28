#include "network/RoomManager.hpp"

#include "network/ClientSession.hpp"
#include "network/JsonProtocol.hpp"
#include "network/Messages.hpp"
#include "network/Room.hpp"

#include <memory>
#include <string>
#include <utility>

RoomManager::RoomManager(
    boost::asio::io_context& ioContext,
    Board boardTemplate,
    PieceSpeedConfig speedConfig,
    MessageRouter::LoginHandler loginHandler
)
    : ioContext_(ioContext),
      boardTemplate_(
          std::move(boardTemplate)
      ),
      speedConfig_(
          std::move(speedConfig)
      ),
      loginHandler_(
          std::move(loginHandler)
      )
{
    rooms_.emplace(
        DEFAULT_ROOM_ID,
        std::make_unique<Room>(
            DEFAULT_ROOM_ID,
            ioContext_,
            boardTemplate_.clone(),
            speedConfig_,
            loginHandler_
        )
    );
}

RoomManager::~RoomManager() = default;

void RoomManager::start()
{
    for (auto& entry : rooms_)
    {
        if (entry.second)
        {
            entry.second->start();
        }
    }
}

void RoomManager::stop()
{
    for (auto& entry : rooms_)
    {
        if (entry.second)
        {
            entry.second->stop();
        }
    }
}

void RoomManager::addToDefaultRoom(
    const SessionPtr& session)
{
    if (!session)
    {
        return;
    }

    removeSession(session);

    const auto found =
        rooms_.find(DEFAULT_ROOM_ID);

    if (
        found == rooms_.end() ||
        !found->second
    )
    {
        const ErrorMessage error{
            "Default room is unavailable"
        };

        session->send(
            JsonProtocol::serializeError(
                error
            )
        );

        return;
    }

    found->second->addSession(session);

    roomIdBySession_[session.get()] =
        DEFAULT_ROOM_ID;
}

void RoomManager::removeSession(
    const SessionPtr& session)
{
    if (!session)
    {
        return;
    }

    const auto found =
        roomIdBySession_.find(
            session.get()
        );

    if (found == roomIdBySession_.end())
    {
        return;
    }

    const auto roomFound =
        rooms_.find(found->second);

    if (
        roomFound != rooms_.end() &&
        roomFound->second
    )
    {
        roomFound->second->removeSession(
            session
        );
    }

    roomIdBySession_.erase(found);
}

void RoomManager::routeMessage(
    const SessionPtr& session,
    const std::string& message)
{
    if (!session)
    {
        return;
    }

    Room* room =
        findRoomForSession(session);

    if (!room)
    {
        const ErrorMessage error{
            "Client is not assigned to a room"
        };

        session->send(
            JsonProtocol::serializeError(
                error
            )
        );

        return;
    }

    room->routeMessage(
        session,
        message
    );
}

Room* RoomManager::findRoomForSession(
    const SessionPtr& session)
{
    if (!session)
    {
        return nullptr;
    }

    const auto found =
        roomIdBySession_.find(
            session.get()
        );

    if (found == roomIdBySession_.end())
    {
        return nullptr;
    }

    const auto roomFound =
        rooms_.find(found->second);

    if (
        roomFound == rooms_.end() ||
        !roomFound->second
    )
    {
        return nullptr;
    }

    return roomFound->second.get();
}

const Room* RoomManager::findRoomForSession(
    const SessionPtr& session) const
{
    if (!session)
    {
        return nullptr;
    }

    const auto found =
        roomIdBySession_.find(
            session.get()
        );

    if (found == roomIdBySession_.end())
    {
        return nullptr;
    }

    const auto roomFound =
        rooms_.find(found->second);

    if (
        roomFound == rooms_.end() ||
        !roomFound->second
    )
    {
        return nullptr;
    }

    return roomFound->second.get();
}