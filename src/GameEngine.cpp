#include "GameEngine.hpp"
#include "Piece.hpp"
#include <cstdlib>

GameEngine::GameEngine(Board board)
    : board_(std::move(board))
{
}

const Board& GameEngine::getBoard() const
{
    return board_;
}

GameState GameEngine::getState() const
{
    return state_;
}

void GameEngine::handleCellClick(int row, int col)
{
    if (state_ == GameState::GAME_OVER)
        return;

    if (!board_.isInsideBoard(row, col))
        return;

    // --- WAITING_SELECTION ---
    if (state_ == GameState::WAITING_SELECTION)
    {
        if (!board_.isEmptyCell(row, col) &&
            !isPieceAtPositionInvolved(row, col))
        {
            selectedCell_ = Position{row, col};
            state_ = GameState::WAITING_TARGET;
        }
        return;
    }

    // --- WAITING_TARGET ---
    if (state_ == GameState::WAITING_TARGET)
    {
        // בדיקה שהכלי הנבחר עדיין קיים
        if (!selectedCell_.has_value() ||
            board_.isEmptyCell(selectedCell_->row, selectedCell_->col))
        {
            selectedCell_ = std::nullopt;
            state_ = GameState::WAITING_SELECTION;
            return;
        }

        // לחיצה על כלי ידידותי אחר — מחליף בחירה
        if (!board_.isEmptyCell(row, col))
        {
            Piece* targetPiece = board_.getCell(row, col);
            Piece* selectedPiece = board_.getCell(selectedCell_->row, selectedCell_->col);

            if (targetPiece->getColor() == selectedPiece->getColor())
            {
                if (!isPieceAtPositionInvolved(row, col))
                {
                    selectedCell_ = Position{row, col};
                }
                return;
            }
        }

        // בדיקת חוקיות דרך RuleEngine
        Piece* selectedPiece = board_.getCell(selectedCell_->row, selectedCell_->col);
        Color playerColor = selectedPiece->getColor();

        MoveValidation validation = ruleEngine_.validateMove(
            board_, *selectedCell_, Position{row, col}, playerColor);

        if (!validation.isValid)
        {
            selectedCell_ = std::nullopt;
            state_ = GameState::WAITING_SELECTION;
            return;
        }

        // התחלת תנועה
        int duration = calculateMoveTime(
            selectedCell_->row, selectedCell_->col, row, col);

        arbiter_.startMove(selectedPiece, *selectedCell_, Position{row, col}, duration);

        // סימון כלי כ-moving — אופציונלי, לצורכי snapshot
        selectedPiece->setState(PieceState::Moving);

        selectedCell_ = std::nullopt;
        state_ = GameState::MOVE_IN_PROGRESS;

        // ייתכן ואין תנועות פעילות (duration=0)
        if (!arbiter_.hasActiveMoves())
        {
            state_ = GameState::WAITING_SELECTION;
        }
        return;
    }

    // --- MOVE_IN_PROGRESS ---
    if (state_ == GameState::MOVE_IN_PROGRESS)
    {
        // לא ניתן לבחור כלי בזמן שתנועה פעילה
        // (אלא אם רוצים לאפשר החלפת בחירה למהלך הבא — כרגע נחכה)
        return;
    }
}

void GameEngine::advanceTime(int milliseconds)
{
    arbiter_.advanceTime(milliseconds);

    // איסוף מהלכים מושלמים, יישום אטומי על הלוח
    auto completed = arbiter_.pollCompletedMoves();
    for (const CompletedMove& cm : completed)
    {
        applyCompletedMove(cm);
        if (state_ == GameState::GAME_OVER)
            return;
    }

    // מעבר חזרה למצב בחירה אם אין תנועות פעילות
    if (!arbiter_.hasActiveMoves() && state_ == GameState::MOVE_IN_PROGRESS)
    {
        state_ = GameState::WAITING_SELECTION;
    }
}

GameSnapshot GameEngine::getSnapshot() const
{
    GameSnapshot snapshot;
    snapshot.boardWidth = board_.colCount();
    snapshot.boardHeight = board_.rowCount();
    snapshot.selectedCell = selectedCell_;
    snapshot.gameOver = (state_ == GameState::GAME_OVER);

    // איסוף כלים מהלוח
    const auto& grid = board_.getGrid();
    for (int r = 0; r < board_.rowCount(); ++r)
    {
        for (int c = 0; c < board_.colCount(); ++c)
        {
            Piece* p = grid[r][c];
            if (p)
            {
                PieceInfo info;
                info.kind = p->getType();
                info.color = p->getColor();
                info.pieceId = p->getId();
                info.cell = p->getCell();
                info.state = p->getState();
                info.progress = 0.0;

                // אם כלי בתנועה — חפש את התנועה הפעילה
                if (p->getState() == PieceState::Moving)
                {
                    for (const Move& move : arbiter_.getActiveMoves())
                    {
                        if (move.getPiece() == p)
                        {
                            info.progress = move.getProgress();
                            break;
                        }
                    }
                }

                snapshot.pieces.push_back(info);
            }
        }
    }

    // זיהוי מנצח — נקבע לפי ה-state הפנימי
    // (אם state_==GAME_OVER, המנצח נקבע ב-applyCompletedMove)
    // לעת עתה, snapshot.winner נשאר nullopt כברירת מחדל
    // יורחב כשתתווסף לוגיקת winner מפורשת

    return snapshot;
}

bool GameEngine::isPieceAtPositionInvolved(int row, int col) const
{
    return arbiter_.isCellInvolved(row, col);
}

int GameEngine::calculateMoveTime(int fromRow, int fromCol,
                                   int toRow, int toCol) const
{
    int distance = std::abs(toRow - fromRow) + std::abs(toCol - fromCol);
    return distance * 1000;
}

void GameEngine::applyCompletedMove(const CompletedMove& cm)
{
    if (cm.wasCancelled)
    {
        // החזרת כלי למצב idle — נשאר בתא המקור
        if (cm.piece)
            cm.piece->setState(PieceState::Idle);
        return;
    }

    // הסרת כלי מהתא המקור
    Piece* piece = board_.takeCell(cm.from.row, cm.from.col);

    if (!piece)
    {
        piece = cm.piece;  // fallback — למקרה שכבר הוזז
    }

    // מחיקת כלי אויב ביעד
    Piece* targetPiece = board_.getCell(cm.to.row, cm.to.col);
    if (targetPiece)
    {
        // לכידת מלך = Game Over
        if (targetPiece->getType() == PieceType::King)
        {
            targetPiece->setState(PieceState::Captured);
            delete board_.takeCell(cm.to.row, cm.to.col);
            board_.setCell(cm.to.row, cm.to.col, piece);
            piece->setCell(cm.to);
            piece->setState(PieceState::Idle);
            state_ = GameState::GAME_OVER;
            return;
        }

        targetPiece->setState(PieceState::Captured);
        delete board_.takeCell(cm.to.row, cm.to.col);
    }

    // הצבת הכלי ביעד
    board_.setCell(cm.to.row, cm.to.col, piece);
    piece->setCell(cm.to);
    piece->setState(PieceState::Idle);
}
