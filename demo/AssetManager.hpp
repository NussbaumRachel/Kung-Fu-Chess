#pragma once

#include "img.hpp"
#include "AnimatedSprite.hpp"
#include <map>
#include <string>
#include "model/types.hpp"
#include "model/Piece.hpp"

/// טוען ומרכז את כל ה-assets: לוח + 12 AnimatedSprites.
/// מחליף את SpriteSheet הישן.
class AssetManager
{
public:
    /// טוען את כל 12 תיקיות הכלים
    /// @param piecesPath תיקיית pieces2 (e.g. "assets/pieces2")
    bool loadAllPieces(const std::string& piecesPath);

    /// טוען תמונת לוח
    bool loadBoard(const std::string& boardPath);

    /// גישה ל-AnimatedSprite של כלי מסוים
    const AnimatedSprite* getSprite(PieceType kind, Color color) const;

    /// תמונת לוח
    const Img& getBoard() const { return boardImg_; }

    /// האם הלוח נטען
    bool hasBoard() const { return boardImg_.is_loaded(); }

    /// ממיר PieceType+Color למפתח תיקייה (e.g. King+White → "KW")
    static std::string pieceKey(PieceType kind, Color color);

private:
    Img boardImg_;
    // מפתח = "KW", "KB", ...
    std::map<std::string, AnimatedSprite> sprites_;
};
