#include "network/JsonProtocol.hpp"
#include "game_engine/GameSnapshot.hpp"

#include <nlohmann/json.hpp>


std::string JsonProtocol::serializeSnapshot(const GameSnapshot& snap)
{
    nlohmann::json j;

    j["type"] = "snapshot";
    j["data"]["status"] = "ok";

    return j.dump();
}
