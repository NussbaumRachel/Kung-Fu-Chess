#pragma once

#include <string>
#include "game_engine/GameSnapshot.hpp"

class JsonProtocol
{
public:
    static std::string serializeSnapshot(const GameSnapshot& snap);
};
