#include "config/PieceSpeedConfig.hpp"
#include "config/PieceConfigReader.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

bool PieceSpeedConfig::load(const std::string& piecesPath)
{
    if (!fs::exists(piecesPath) || !fs::is_directory(piecesPath))
        return false;

    for (const auto& entry : fs::directory_iterator(piecesPath))
    {
        if (!entry.is_directory()) continue;

        std::string key = entry.path().filename().string();
        if (key.size() != 2) continue;

        // קריאת move speed
        std::string moveCfg = piecesPath + "/" + key + "/states/move/config.json";
        if (fs::exists(moveCfg))
            moveSpeeds_[key] = readSpeed(moveCfg);

        // קריאת jump speed
        std::string jumpCfg = piecesPath + "/" + key + "/states/jump/config.json";
        if (fs::exists(jumpCfg))
            jumpSpeeds_[key] = readSpeed(jumpCfg);
    }

    return !moveSpeeds_.empty();
}

double PieceSpeedConfig::getMoveSpeed(PieceType type, Color color) const
{
    std::string key = pieceKey(type, color);
    auto it = moveSpeeds_.find(key);
    return (it != moveSpeeds_.end()) ? it->second : 1.0;
}

double PieceSpeedConfig::getJumpSpeed(PieceType type, Color color) const
{
    std::string key = pieceKey(type, color);
    auto it = jumpSpeeds_.find(key);
    return (it != jumpSpeeds_.end()) ? it->second : 3.0;
}

std::string PieceSpeedConfig::pieceKey(PieceType type, Color color) const
{
    std::string key;
    switch (type)
    {
        case PieceType::King:   key = "K"; break;
        case PieceType::Queen:  key = "Q"; break;
        case PieceType::Rook:   key = "R"; break;
        case PieceType::Bishop: key = "B"; break;
        case PieceType::Knight: key = "N"; break;
        case PieceType::Pawn:   key = "P"; break;
    }
    key += (color == Color::White) ? "W" : "B";
    return key;
}

double PieceSpeedConfig::readSpeed(const std::string& configPath) const
{
    return PieceConfigReader::readDouble(configPath, "speed_m_per_sec", 1.0);
}
