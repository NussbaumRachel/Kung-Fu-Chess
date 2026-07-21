#pragma once

#include <string>
#include "game_engine/GameSnapshot.hpp"
#include <nlohmann/json.hpp>

class JsonProtocol
{
public:
    static std::string serializeSnapshot(const GameSnapshot& snap);

    // message helpers
    static std::string makeClickMsg(int row, int col);
    static bool        isClickMsg(const nlohmann::json& j, int& outRow, int& outCol);
};
