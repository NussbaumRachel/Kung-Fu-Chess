#pragma once

#include <vector>
#include <optional>
#include "GameSnapshot.hpp"
#include "Position.hpp"
#include "types.hpp"

/// סימולציית לוגיקה מינימלית — מחליף זמני של GameEngine.
///
/// אחראי על:
///   - ניהול מצב לוח פשוט (מיקומי כלים)
///   - טיפול בקליקים (סימון / ביטול / הזזה)
///   - קידום אנימציות (advanceTime)
///   - הפקת GameSnapshot ל-Renderer
///
/// בשלב האינטגרציה — מחליפים את המחלקה הזו ב-GameEngine/GameController.
class DemoBoardState
{
public:
    DemoBoardState();

    /// טוען לוח התחלתי (כלים בעמדות פתיחה)
    void loadDefaultBoard();

    /// טיפול בקליק על תא
    void handleClick(int row, int col);

    /// קידום זמן אנימציה
    void advanceTime(int milliseconds);

    /// הפקת תמונת מצב נוכחית
    GameSnapshot getSnapshot() const;

private:
    int boardWidth_  = 8;
    int boardHeight_ = 8;

    std::vector<PieceInfo> pieces_;

    std::optional<Position> selectedCell_;
    int nextPieceId_ = 1;

    // ── אנימציה פעילה ──
    struct AnimState
    {
        int  pieceId;
        Position from;
        Position to;
        int  elapsedMs   = 0;
        int  durationMs  = 500;
        bool active      = false;
    };
    std::optional<AnimState> activeAnim_;

    // ── עזרים ──
    PieceInfo* findPieceAt(int row, int col);
    const PieceInfo* findPieceAt(int row, int col) const;
    bool isEmpty(int row, int col) const;
    PieceInfo makePiece(PieceType kind, Color color, int row, int col);
};
