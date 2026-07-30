#pragma once

#include "config/PieceSpeedConfig.hpp"
#include "model/Board.hpp"
#include "network/Messages.hpp"
#include "network/RoomOperationResult.hpp"

#include <boost/asio.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>

class ClientSession;
class Room;
class IGameResultRepository;
class RoomManager
{
public:
    using SessionPtr =
        std::shared_ptr<ClientSession>;

RoomManager(
    boost::asio::io_context& ioContext,
    Board boardTemplate,
    PieceSpeedConfig speedConfig,
    IGameResultRepository& gameResultRepository
);
    ~RoomManager();

    RoomManager(const RoomManager&) = delete;
    RoomManager& operator=(const RoomManager&) = delete;

    RoomManager(RoomManager&&) = delete;
    RoomManager& operator=(RoomManager&&) = delete;

    void start();

    void stop();

    void addToDefaultRoom(
        const SessionPtr& session
    );

    [[nodiscard]]
    RoomOperationResult createRoom(
        const SessionPtr& session,
        const std::string& roomId
    );

    [[nodiscard]]
    RoomOperationResult joinRoom(
        const SessionPtr& session,
        const std::string& roomId
    );

    [[nodiscard]]
    RoomOperationResult leaveRoom(
        const SessionPtr& session
    );

    void removeSession(
        const SessionPtr& session
    );

    void handleClick(
        const SessionPtr& session,
        const ClickMessage& click
    );

    [[nodiscard]]
    bool roomExists(
        const std::string& roomId
    ) const;

    [[nodiscard]]
    std::size_t roomCount() const;

    [[nodiscard]]
    std::string roomIdForSession(
        const SessionPtr& session
    ) const;

private:
    [[nodiscard]]
    bool createRoomInternal(
        const std::string& roomId
    );

    [[nodiscard]]
    bool moveSessionToRoom(
        const SessionPtr& session,
        const std::string& roomId
    );

    void removeSessionFromCurrentRoom(
        const SessionPtr& session,
        bool removeEmptyRoom
    );

    void removeRoomIfEmpty(
        const std::string& roomId
    );

    [[nodiscard]]
    Room* findRoomForSession(
        const SessionPtr& session
    );

    [[nodiscard]]
    const Room* findRoomForSession(
        const SessionPtr& session
    ) const;

    [[nodiscard]]
    static bool isValidRoomId(
        const std::string& roomId
    );

    void sendError(
        const SessionPtr& session,
        const std::string& errorMessage
    ) const;

private:
    static constexpr const char*
        DEFAULT_ROOM_ID = "default";

    boost::asio::io_context& ioContext_;
    IGameResultRepository&
        gameResultRepository_;
    Board boardTemplate_;

    PieceSpeedConfig speedConfig_;

    bool started_ = false;

    std::unordered_map<
        std::string,
        std::unique_ptr<Room>
    > rooms_;

    std::unordered_map<
        ClientSession*,
        std::string
    > roomIdBySession_;
};