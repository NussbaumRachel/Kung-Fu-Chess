#pragma once

#include "network/Messages.hpp"

#include <string>

class JsonProtocol
{
public:
    [[nodiscard]]
    static MessageType getMessageType(
        const std::string& jsonText
    );

    [[nodiscard]]
    static std::string serializeWelcome(
        const WelcomeMessage& message
    );

    [[nodiscard]]
    static WelcomeMessage deserializeWelcome(
        const std::string& jsonText
    );

    [[nodiscard]]
    static std::string serializeLogin(
        const LoginMessage& message
    );

    [[nodiscard]]
    static LoginMessage deserializeLogin(
        const std::string& jsonText
    );

    [[nodiscard]]
    static std::string serializeLoginResult(
        const LoginResultMessage& message
    );

    [[nodiscard]]
    static LoginResultMessage deserializeLoginResult(
        const std::string& jsonText
    );

    [[nodiscard]]
    static std::string serializeClick(
        const ClickMessage& message
    );

    [[nodiscard]]
    static ClickMessage deserializeClick(
        const std::string& jsonText
    );

    [[nodiscard]]
    static std::string serializeSnapshot(
        const GameSnapshot& snapshot
    );

    [[nodiscard]]
    static SnapshotMessage deserializeSnapshot(
        const std::string& jsonText
    );

    [[nodiscard]]
    static std::string serializeCreateRoom(
        const CreateRoomMessage& message
    );

    [[nodiscard]]
    static CreateRoomMessage deserializeCreateRoom(
        const std::string& jsonText
    );

    [[nodiscard]]
    static std::string serializeJoinRoom(
        const JoinRoomMessage& message
    );

    [[nodiscard]]
    static JoinRoomMessage deserializeJoinRoom(
        const std::string& jsonText
    );

    [[nodiscard]]
    static std::string serializeLeaveRoom(
        const LeaveRoomMessage& message
    );

    [[nodiscard]]
    static LeaveRoomMessage deserializeLeaveRoom(
        const std::string& jsonText
    );

    [[nodiscard]]
    static std::string serializeRoomResult(
        const RoomResultMessage& message
    );

    [[nodiscard]]
    static RoomResultMessage deserializeRoomResult(
        const std::string& jsonText
    );

    [[nodiscard]]
    static std::string serializeError(
        const ErrorMessage& message
    );

    [[nodiscard]]
    static ErrorMessage deserializeError(
        const std::string& jsonText
    );
};