#pragma once

#include "img.hpp"
#include <unordered_map>
#include "AssetManager.hpp"
#include "AnimationManager.hpp"
#include "game_engine/GameSnapshot.hpp"

/// מצייר כלים (עם אנימציות frame-based). עובד עם AssetManager + AnimationManager.
class PieceRenderer
{
public:
    explicit PieceRenderer(const AssetManager& assets);

    void drawPieces(Img& canvas, const GameSnapshot& snapshot, int cellSize,
                    AnimationManager& animMgr);

private:
    const AssetManager* assets_;  // לא הבעלים — ChessRenderer הוא הבעלים (unique_ptr)
    // mutable std::unordered_map<int, Position> previousCells_;

    cv::Point computePiecePixel(const PieceInfo& info, int cellSize) const;
    void drawSpriteCentered(Img& canvas, const Img& sprite,
                            const cv::Point& center, int cellSize) const;
};
