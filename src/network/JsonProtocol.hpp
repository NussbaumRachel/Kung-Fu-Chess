#pragma once

#include <string>

struct GameSnapshot;

class JsonProtocol
{
public:
    static std::string serializeSnapshot(const GameSnapshot& snap);
};
