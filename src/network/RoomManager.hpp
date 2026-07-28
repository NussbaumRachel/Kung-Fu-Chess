#pragma once

#include "config/PieceSpeedConfig.hpp"
#include "model/Board.hpp"
#include "network/Messages.hpp"

#include <boost/asio.hpp>

#include <memory>
#include <string>
#include <unordered_map>

class ClientSession;
class Room;

class RoomManager
{
public:
    using SessionPtr =
        std::shared_ptr<ClientSession>;

    RoomManager(
        boost::asio::io_context& ioContext,
        Board boardTemplate,
        PieceSpeedConfig speedConfig
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

    void removeSession(
        const SessionPtr& session
    );

    void handleClick(
        const SessionPtr& session,
        const ClickMessage& click
    );

private:
    Room* findRoomForSession(
        const SessionPtr& session
    );

    const Room* findRoomForSession(
        const SessionPtr& session
    ) const;

    void sendError(
        const SessionPtr& session,
        const std::string& errorMessage
    ) const;

private:
    static constexpr const char*
        DEFAULT_ROOM_ID = "default";

    boost::asio::io_context& ioContext_;

    Board boardTemplate_;

    PieceSpeedConfig speedConfig_;

    std::unordered_map<
        std::string,
        std::unique_ptr<Room>
    > rooms_;

    std::unordered_map<
        ClientSession*,
        std::string
    > roomIdBySession_;
};