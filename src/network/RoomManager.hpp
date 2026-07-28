#pragma once

#include "config/PieceSpeedConfig.hpp"
#include "model/Board.hpp"
#include "network/MessageRouter.hpp"

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
        PieceSpeedConfig speedConfig,
        MessageRouter::LoginHandler loginHandler
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

    void routeMessage(
        const SessionPtr& session,
        const std::string& message
    );

private:
    Room* findRoomForSession(
        const SessionPtr& session
    );

    const Room* findRoomForSession(
        const SessionPtr& session
    ) const;

private:
    static constexpr const char*
        DEFAULT_ROOM_ID = "default";

    boost::asio::io_context& ioContext_;

    Board boardTemplate_;

    PieceSpeedConfig speedConfig_;

    MessageRouter::LoginHandler loginHandler_;

    std::unordered_map<
        std::string,
        std::unique_ptr<Room>
    > rooms_;

    std::unordered_map<
        ClientSession*,
        std::string
    > roomIdBySession_;
};