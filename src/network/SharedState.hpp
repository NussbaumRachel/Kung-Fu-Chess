#pragma once

#include <mutex>
#include <queue>
#include <string>

struct SharedState
{
    std::mutex mtx;

    std::queue<std::string> incomingMessages;

    bool connected = false;
    bool running = true;
    bool authenticated = false;

    std::string username;
    std::string currentRoomId = "default";

    std::mutex outgoingMtx;

    std::queue<std::string> outgoingMessages;
};