#pragma once

#include "game_engine/GameSnapshot.hpp"

#include <string>

enum class MessageType
{
    Welcome,
    Login,
    LoginResult,
    Click,
    Snapshot,
    Error,
    Unknown
};

struct WelcomeMessage
{
    std::string color;
};

struct LoginMessage
{
    std::string username;
};

struct LoginResultMessage
{
    bool success = false;
    std::string username;
    std::string message;
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