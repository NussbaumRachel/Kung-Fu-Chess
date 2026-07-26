#pragma once

#include "game_engine/GameSnapshot.hpp"

#include <string>
#include <utility>

enum class MessageType
{
    Welcome,
    Click,
    Snapshot,
    Error,
    Unknown
};

struct WelcomeMessage
{
    std::string color;
};

struct ClickMessage
{
    int row = -1;
    int col = -1;
};

struct SnapshotMessage
{
    GameSnapshot snapshot;
};

struct ErrorMessage
{
    std::string message;
};