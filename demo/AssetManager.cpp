#include "AssetManager.hpp"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

// ════════════════════════════════════════════════
//  pieceKey
// ════════════════════════════════════════════════

std::string AssetManager::pieceKey(PieceType kind, Color color)
{
    std::string key;
    switch (kind)
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

// ════════════════════════════════════════════════
//  loadAllPieces
// ════════════════════════════════════════════════

bool AssetManager::loadAllPieces(const std::string& piecesPath)
{
    if (!fs::exists(piecesPath))
    {
        std::cerr << "[AssetManager] Pieces folder not found: " << piecesPath << '\n';
        return false;
    }

    int loaded = 0;

    // מעבר על כל 12 תתי-התיקיות (KW, KB, QW, QB, ...)
    for (const auto& entry : fs::directory_iterator(piecesPath))
    {
        if (!entry.is_directory()) continue;

        std::string key = entry.path().filename().string();
        // מוודא שהמפתח תקין (2 תווים, אות ראשונה K/Q/R/B/N/P, שנייה W/B)
        if (key.size() != 2) continue;

        AnimatedSprite sprite;
        if (sprite.load(entry.path().string()))
        {
            sprites_[key] = std::move(sprite);
            ++loaded;
        }
    }

    std::cout << "[AssetManager] Loaded " << loaded << "/12 pieces\n";
    return loaded > 0;
}

// ════════════════════════════════════════════════
//  loadBoard
// ════════════════════════════════════════════════

bool AssetManager::loadBoard(const std::string& boardPath)
{
    try
    {
        boardImg_.read(boardPath);
        std::cout << "[AssetManager] Board loaded: " << boardPath
                  << " (" << boardImg_.width() << 'x' << boardImg_.height() << ")\n";
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[AssetManager] Cannot load board: " << boardPath
                  << " (" << e.what() << ")\n";
        return false;
    }
}

// ════════════════════════════════════════════════
//  getSprite
// ════════════════════════════════════════════════

const AnimatedSprite* AssetManager::getSprite(PieceType kind, Color color) const
{
    std::string key = pieceKey(kind, color);
    auto it = sprites_.find(key);
    if (it != sprites_.end())
        return &it->second;
    return nullptr;
}
