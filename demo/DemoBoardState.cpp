#include "DemoBoardState.hpp"
#include "DemoConfig.hpp"
#include <algorithm>
#include <iostream>

DemoBoardState::DemoBoardState()
{
    loadDefaultBoard();
}

// ════════════════════════════════════════════════════════════════
//  לוח התחלתי — עמדות פתיחה של שחמט
// ════════════════════════════════════════════════════════════════

void DemoBoardState::loadDefaultBoard()
{
    pieces_.clear();

    // שורה אחורית — שחור (שורה 0)
    pieces_.push_back(makePiece(PieceType::Rook,   Color::Black, 0, 0));
    pieces_.push_back(makePiece(PieceType::Knight, Color::Black, 0, 1));
    pieces_.push_back(makePiece(PieceType::Bishop, Color::Black, 0, 2));
    pieces_.push_back(makePiece(PieceType::Queen,  Color::Black, 0, 3));
    pieces_.push_back(makePiece(PieceType::King,   Color::Black, 0, 4));
    pieces_.push_back(makePiece(PieceType::Bishop, Color::Black, 0, 5));
    pieces_.push_back(makePiece(PieceType::Knight, Color::Black, 0, 6));
    pieces_.push_back(makePiece(PieceType::Rook,   Color::Black, 0, 7));

    // רגלים — שחור (שורה 1)
    for (int c = 0; c < 8; ++c)
        pieces_.push_back(makePiece(PieceType::Pawn, Color::Black, 1, c));

    // רגלים — לבן (שורה 6)
    for (int c = 0; c < 8; ++c)
        pieces_.push_back(makePiece(PieceType::Pawn, Color::White, 6, c));

    // שורה אחורית — לבן (שורה 7)
    pieces_.push_back(makePiece(PieceType::Rook,   Color::White, 7, 0));
    pieces_.push_back(makePiece(PieceType::Knight, Color::White, 7, 1));
    pieces_.push_back(makePiece(PieceType::Bishop, Color::White, 7, 2));
    pieces_.push_back(makePiece(PieceType::Queen,  Color::White, 7, 3));
    pieces_.push_back(makePiece(PieceType::King,   Color::White, 7, 4));
    pieces_.push_back(makePiece(PieceType::Bishop, Color::White, 7, 5));
    pieces_.push_back(makePiece(PieceType::Knight, Color::White, 7, 6));
    pieces_.push_back(makePiece(PieceType::Rook,   Color::White, 7, 7));

    selectedCell_ = std::nullopt;
    activeAnim_   = std::nullopt;

    std::cout << "[DemoBoardState] Default board loaded ("
              << pieces_.size() << " pieces)\n";
}

// ════════════════════════════════════════════════════════════════
//  עזרים
// ════════════════════════════════════════════════════════════════

PieceInfo DemoBoardState::makePiece(PieceType kind, Color color, int row, int col)
{
    PieceInfo info;
    info.kind     = kind;
    info.color    = color;
    info.pieceId  = nextPieceId_++;
    info.cell     = {row, col};
    info.state    = PieceState::Idle;
    info.progress = 0.0;
    return info;
}

PieceInfo* DemoBoardState::findPieceAt(int row, int col)
{
    for (auto& p : pieces_)
    {
        if (p.state != PieceState::Captured &&
            p.cell.row == row && p.cell.col == col)
        {
            return &p;
        }
    }
    return nullptr;
}

const PieceInfo* DemoBoardState::findPieceAt(int row, int col) const
{
    for (auto& p : pieces_)
    {
        if (p.state != PieceState::Captured &&
            p.cell.row == row && p.cell.col == col)
        {
            return &p;
        }
    }
    return nullptr;
}

bool DemoBoardState::isEmpty(int row, int col) const
{
    return findPieceAt(row, col) == nullptr;
}

// ════════════════════════════════════════════════════════════════
//  handleClick — לוגיקת קליק פשוטה
// ════════════════════════════════════════════════════════════════

void DemoBoardState::handleClick(int row, int col)
{
    // מתעלם אם אנימציה רצה
    if (activeAnim_.has_value())
        return;

    PieceInfo* clicked = findPieceAt(row, col);
    bool cellEmpty = (clicked == nullptr);

    // ── מצב 0: שום כלי לא נבחר ──
    if (!selectedCell_.has_value())
    {
        if (cellEmpty)
            return;  // קליק על תא ריק — כלום

        // בחר כלי
        selectedCell_ = {row, col};
        std::cout << "[DemoBoardState] Selected piece at " << row << ',' << col << '\n';
        return;
    }

    // ── מצב 1: יש כלי נבחר ──
    int selRow = selectedCell_->row;
    int selCol = selectedCell_->col;

    // קליק על אותו תא — בטל בחירה
    if (selRow == row && selCol == col)
    {
        selectedCell_ = std::nullopt;
        std::cout << "[DemoBoardState] Deselected\n";
        return;
    }

    PieceInfo* selected = findPieceAt(selRow, selCol);
    if (!selected)
    {
        // הכלי הנבחר נעלם (לא אמור לקרות בדמו)
        selectedCell_ = std::nullopt;
        return;
    }

    // קליק על כלי אחר מאותו צבע — החלף בחירה
    if (!cellEmpty && clicked->color == selected->color)
    {
        selectedCell_ = {row, col};
        std::cout << "[DemoBoardState] Switched selection to " << row << ',' << col << '\n';
        return;
    }

    // ── צעד 2: בצע מהלך (לכיוון תא ריק או תא אויב) ──
    // בדמו: כל מהלך חוקי (מגבלה: רק משבצת אחת)
    bool isAdjacent = std::abs(row - selRow) <= 1 && std::abs(col - selCol) <= 1;
    if (!isAdjacent)
    {
        // בחר תא חדש? או בטל?
        selectedCell_ = std::nullopt;
        return;
    }

    // הסר כלי אויב בתא היעד
    if (!cellEmpty)
    {
        clicked->state = PieceState::Captured;
    }

    // התחל אנימציה
    AnimState anim;
    anim.pieceId    = selected->pieceId;
    anim.from       = selected->cell;
    anim.to         = {row, col};
    anim.elapsedMs  = 0;
    anim.durationMs = DemoConfig::MOVE_DURATION_MS;
    anim.active     = true;
    activeAnim_     = anim;

    // עדכן מצב כלי
    selected->state    = PieceState::Moving;
    selected->progress = 0.0;

    selectedCell_ = std::nullopt;

    std::cout << "[DemoBoardState] Move from " << selRow << ',' << selCol
              << " to " << row << ',' << col << '\n';
}

// ════════════════════════════════════════════════════════════════
//  advanceTime — קידום אנימציה
// ════════════════════════════════════════════════════════════════

void DemoBoardState::advanceTime(int milliseconds)
{
    if (!activeAnim_.has_value())
        return;

    activeAnim_->elapsedMs += milliseconds;

    double progress = static_cast<double>(activeAnim_->elapsedMs)
                      / activeAnim_->durationMs;
    if (progress > 1.0) progress = 1.0;

    // עדכון progress על הכלי
    for (auto& p : pieces_)
    {
        if (p.pieceId == activeAnim_->pieceId && p.state == PieceState::Moving)
        {
            p.progress = progress;

            if (progress >= 1.0)
            {
                // סיום אנימציה
                p.cell     = activeAnim_->to;
                p.state    = PieceState::Idle;
                p.progress = 0.0;
                activeAnim_ = std::nullopt;
            }
            break;
        }
    }
}

// ════════════════════════════════════════════════════════════════
//  getSnapshot
// ════════════════════════════════════════════════════════════════

GameSnapshot DemoBoardState::getSnapshot() const
{
    GameSnapshot snap;
    snap.boardWidth   = boardWidth_;
    snap.boardHeight  = boardHeight_;
    snap.pieces       = pieces_;
    snap.selectedCell = selectedCell_;
    snap.gameOver     = false;
    snap.winner       = std::nullopt;
    return snap;
}
