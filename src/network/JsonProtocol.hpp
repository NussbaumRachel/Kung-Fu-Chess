#pragma once

#include "network/Messages.hpp"

#include <string>

class JsonProtocol
{
public:
    [[nodiscard]]
    static MessageType getMessageType(const std::string& jsonText);

    [[nodiscard]]
    static std::string serializeWelcome(const WelcomeMessage& message);

    [[nodiscard]]
    static WelcomeMessage deserializeWelcome(const std::string& jsonText);

    [[nodiscard]]
    static std::string serializeClick(const ClickMessage& message);

    [[nodiscard]]
    static ClickMessage deserializeClick(const std::string& jsonText);

    [[nodiscard]]
    static std::string serializeSnapshot(const GameSnapshot& snapshot);

    [[nodiscard]]
    static SnapshotMessage deserializeSnapshot(const std::string& jsonText);

    [[nodiscard]]
    static std::string serializeError(const ErrorMessage& message);

    [[nodiscard]]
    static ErrorMessage deserializeError(const std::string& jsonText);
};