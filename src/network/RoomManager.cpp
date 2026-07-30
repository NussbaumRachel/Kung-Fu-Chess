#include "network/RoomManager.hpp"
#include "network/ClientSession.hpp"
#include "network/JsonProtocol.hpp"
#include "game_result/IGameResultRepository.hpp"
#include "network/Room.hpp"
#include <cctype>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <stdexcept>
RoomManager::RoomManager(
    boost::asio::io_context& ioContext,
    Board boardTemplate,
    PieceSpeedConfig speedConfig,
    IGameResultRepository& gameResultRepository
)
    : ioContext_(ioContext),
      gameResultRepository_(
          gameResultRepository
      ),
      boardTemplate_(
          std::move(boardTemplate)
      ),
      speedConfig_(
          std::move(speedConfig)
      )
{
    if (!createRoomInternal(DEFAULT_ROOM_ID))
    {
        throw std::runtime_error(
            "Failed to create default room"
        );
    }
}
RoomManager::~RoomManager() = default;

void RoomManager::start()
{
    if (started_)
    {
        return;
    }

    started_ = true;

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
    if (!started_)
    {
        return;
    }

    for (auto& entry : rooms_)
    {
        if (entry.second)
        {
            entry.second->stop();
        }
    }

    started_ = false;
}

void RoomManager::addToDefaultRoom(
    const SessionPtr& session)
{
    if (!session)
    {
        return;
    }

    const RoomOperationResult result =
        joinRoom(
            session,
            DEFAULT_ROOM_ID
        );

    if (!result.success)
    {
        sendError(
            session,
            result.message
        );
    }
}

RoomOperationResult RoomManager::createRoom(
    const SessionPtr& session,
    const std::string& roomId)
{
    if (!session)
    {
        return {
            false,
            "Invalid client session"
        };
    }

    if (!isValidRoomId(roomId))
    {
        return {
            false,
            "Room ID must contain 1 to 32 letters, digits, underscores or hyphens"
        };
    }

    if (roomExists(roomId))
    {
        return {
            false,
            "Room already exists"
        };
    }

    if (!createRoomInternal(roomId))
    {
        return {
            false,
            "Failed to create room"
        };
    }

    if (!moveSessionToRoom(session, roomId))
    {
        removeRoomIfEmpty(roomId);

        return {
            false,
            "Room was created but the client could not join it"
        };
    }

    std::cout
        << "Room '"
        << roomId
        << "' created"
        << std::endl;

    return {
        true,
        "Room created"
    };
}

RoomOperationResult RoomManager::joinRoom(
    const SessionPtr& session,
    const std::string& roomId)
{
    if (!session)
    {
        return {
            false,
            "Invalid client session"
        };
    }

    if (!isValidRoomId(roomId))
    {
        return {
            false,
            "Invalid room ID"
        };
    }

    if (!roomExists(roomId))
    {
        return {
            false,
            "Room does not exist"
        };
    }

    const std::string currentRoomId =
        roomIdForSession(session);

    if (currentRoomId == roomId)
    {
        return {
            true,
            "Client is already in this room"
        };
    }

    if (!moveSessionToRoom(session, roomId))
    {
        return {
            false,
            "Failed to join room"
        };
    }

    return {
        true,
        "Room joined"
    };
}

RoomOperationResult RoomManager::leaveRoom(
    const SessionPtr& session)
{
    if (!session)
    {
        return {
            false,
            "Invalid client session"
        };
    }

    const auto found =
        roomIdBySession_.find(
            session.get()
        );

    if (found == roomIdBySession_.end())
    {
        return {
            false,
            "Client is not assigned to a room"
        };
    }

    removeSessionFromCurrentRoom(
        session,
        true
    );

    return {
        true,
        "Room left"
    };
}

void RoomManager::removeSession(
    const SessionPtr& session)
{
    if (!session)
    {
        return;
    }

    removeSessionFromCurrentRoom(
        session,
        true
    );
}

void RoomManager::handleClick(
    const SessionPtr& session,
    const ClickMessage& click)
{
    if (!session)
    {
        return;
    }

    Room* room =
        findRoomForSession(session);

    if (!room)
    {
        sendError(
            session,
            "Client is not assigned to a room"
        );

        return;
    }

    room->handleClick(
        session,
        click
    );
}

bool RoomManager::roomExists(
    const std::string& roomId) const
{
    const auto found =
        rooms_.find(roomId);

    return
        found != rooms_.end() &&
        found->second != nullptr;
}

std::size_t RoomManager::roomCount() const
{
    return rooms_.size();
}

std::string RoomManager::roomIdForSession(
    const SessionPtr& session) const
{
    if (!session)
    {
        return {};
    }

    const auto found =
        roomIdBySession_.find(
            session.get()
        );

    if (found == roomIdBySession_.end())
    {
        return {};
    }

    return found->second;
}

bool RoomManager::createRoomInternal(
    const std::string& roomId)
{
    if (roomExists(roomId))
    {
        return false;
    }

    auto room =
        std::make_unique<Room>(
            roomId,
            ioContext_,
            boardTemplate_.clone(),
            speedConfig_,
            gameResultRepository_
        );
    if (started_)
    {
        room->start();
    }

    const auto inserted =
        rooms_.emplace(
            roomId,
            std::move(room)
        );

    return inserted.second;
}

bool RoomManager::moveSessionToRoom(
    const SessionPtr& session,
    const std::string& roomId)
{
    if (!session)
    {
        return false;
    }

    const auto targetFound =
        rooms_.find(roomId);

    if (
        targetFound == rooms_.end() ||
        !targetFound->second
    )
    {
        return false;
    }

    const std::string previousRoomId =
        roomIdForSession(session);

    /*
     * חדר היעד נבדק לפני הסרת המשתמש מהחדר הקודם.
     * כך כישלון בחיפוש החדר אינו משאיר את המשתמש ללא חדר.
     */
    removeSessionFromCurrentRoom(
        session,
        false
    );

    targetFound->second->addSession(
        session
    );

    roomIdBySession_[session.get()] =
        roomId;

    if (
        !previousRoomId.empty() &&
        previousRoomId != roomId
    )
    {
        removeRoomIfEmpty(
            previousRoomId
        );
    }

    return true;
}

void RoomManager::removeSessionFromCurrentRoom(
    const SessionPtr& session,
    bool removeEmptyRoom)
{
    if (!session)
    {
        return;
    }

    const auto mappingFound =
        roomIdBySession_.find(
            session.get()
        );

    if (mappingFound == roomIdBySession_.end())
    {
        return;
    }

    const std::string roomId =
        mappingFound->second;

    const auto roomFound =
        rooms_.find(roomId);

    if (
        roomFound != rooms_.end() &&
        roomFound->second
    )
    {
        roomFound->second->removeSession(
            session
        );
    }

    roomIdBySession_.erase(
        mappingFound
    );

    if (removeEmptyRoom)
    {
        removeRoomIfEmpty(
            roomId
        );
    }
}

void RoomManager::removeRoomIfEmpty(
    const std::string& roomId)
{
    /*
     * חדר ברירת המחדל נשאר קיים גם כשהוא ריק,
     * משום שחיבורים חדשים משויכים אליו אוטומטית.
     */
    if (roomId == DEFAULT_ROOM_ID)
    {
        return;
    }

    const auto found =
        rooms_.find(roomId);

    if (
        found == rooms_.end() ||
        !found->second ||
        !found->second->empty()
    )
    {
        return;
    }

    if (started_)
    {
        found->second->stop();
    }

    rooms_.erase(found);

    std::cout
        << "Empty room '"
        << roomId
        << "' removed"
        << std::endl;
}

Room* RoomManager::findRoomForSession(
    const SessionPtr& session)
{
    if (!session)
    {
        return nullptr;
    }

    const auto mappingFound =
        roomIdBySession_.find(
            session.get()
        );

    if (mappingFound == roomIdBySession_.end())
    {
        return nullptr;
    }

    const auto roomFound =
        rooms_.find(
            mappingFound->second
        );

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

    const auto mappingFound =
        roomIdBySession_.find(
            session.get()
        );

    if (mappingFound == roomIdBySession_.end())
    {
        return nullptr;
    }

    const auto roomFound =
        rooms_.find(
            mappingFound->second
        );

    if (
        roomFound == rooms_.end() ||
        !roomFound->second
    )
    {
        return nullptr;
    }

    return roomFound->second.get();
}

bool RoomManager::isValidRoomId(
    const std::string& roomId)
{
    if (
        roomId.empty() ||
        roomId.size() > 32
    )
    {
        return false;
    }

    for (const unsigned char character :
         roomId)
    {
        const bool valid =
            std::isalnum(character) != 0 ||
            character == '_' ||
            character == '-';

        if (!valid)
        {
            return false;
        }
    }

    return true;
}

void RoomManager::sendError(
    const SessionPtr& session,
    const std::string& errorMessage) const
{
    if (!session)
    {
        return;
    }

    const ErrorMessage error{
        errorMessage
    };

    session->send(
        JsonProtocol::serializeError(
            error
        )
    );
}