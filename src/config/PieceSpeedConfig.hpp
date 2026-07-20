#pragma once

#include "model/types.hpp"
#include <string>
#include <unordered_map>
class PieceSpeedConfig
{
public:
    bool load(const std::string& piecesPath);

    double getMoveSpeed(PieceType type, Color color) const;
    double getJumpSpeed(PieceType type, Color color) const;

    bool isLoaded() const { return !moveSpeeds_.empty(); }

private:
    std::unordered_map<std::string, double> moveSpeeds_;  // "KW" → 1.5
    std::unordered_map<std::string, double> jumpSpeeds_;  // "KW" → 3.0

    std::string pieceKey(PieceType type, Color color) const;
    double readSpeed(const std::string& configPath) const;
};
