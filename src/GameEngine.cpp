#include "GameEngine.hpp"
#include "Piece.hpp"
#include "PieceFactory.hpp"
#include <cstdlib>

GameEngine::GameEngine(Board board)
    : board_(std::move(board)),
      boardController_(board_)
{
}

const Board& GameEngine::getBoard() const
{
    return board_;
}

GameState GameEngine::getState() const
{
    return stateMachine_.getState();
}

void GameEngine::handleCellClick(int row, int col)
{
    if (!board_.isInsideBoard(row, col))
        return;

    // ── 1. איסוף מידע ──
    GameState currentState = stateMachine_.getState();
    const auto& selectedCell = stateMachine_.getSelectedCell();

    bool cellIsEmpty = board_.isEmptyCell(row, col);
    bool cellHasFriendlyPiece = false;
    bool cellIsInvolved = arbiter_.isPieceInvolved(row, col);
    bool moveIsValid = false;

    // זיהוי כלי ידידותי (רק כשיש כלי בתא הנוכחי ויש כלי נבחר)
    if (!cellIsEmpty)
    {
        Piece* clickedPiece = board_.getCell(row, col);
        if (selectedCell.has_value() &&
            board_.isInsideBoard(selectedCell->row, selectedCell->col) &&
            !board_.isEmptyCell(selectedCell->row, selectedCell->col))
        {
            Piece* selectedPiece = board_.getCell(selectedCell->row, selectedCell->col);
            if (selectedPiece && clickedPiece &&
                clickedPiece->getColor() == selectedPiece->getColor())
            {
                cellHasFriendlyPiece = true;
            }
        }
    }

    // אימות מהלך — רק כשצריך (מצב WAITING_TARGET, יש בחירה, לא קפיצה/החלפה)
    if (currentState == GameState::WAITING_TARGET &&
        selectedCell.has_value() &&
        board_.isInsideBoard(selectedCell->row, selectedCell->col) &&
        !board_.isEmptyCell(selectedCell->row, selectedCell->col))
    {
        Position clickPos{row, col};

        // לא צריך לאמת מהלך עבור קפיצה או החלפת כלי
        if (!(clickPos == *selectedCell) && !cellHasFriendlyPiece)
        {
            Piece* selectedPiece = board_.getCell(selectedCell->row, selectedCell->col);
            if (selectedPiece)
            {
                MoveValidation validation = ruleEngine_.validateMove(
                    board_, *selectedCell, clickPos, selectedPiece->getColor());
                moveIsValid = validation.isValid;
            }
        }
    }

    // ── 2. שאילת מכונת המצבים ──
    GameDecision d = stateMachine_.evaluate(
        currentState, row, col, selectedCell,
        cellIsEmpty, cellHasFriendlyPiece,
        cellIsInvolved, moveIsValid);

    // ── 3. ביצוע ההחלטה ──
    switch (d.action)
    {
    case ActionType::SelectPiece:
    case ActionType::SwitchPiece:
        stateMachine_.setSelectedCell(d.selectedCell);
        stateMachine_.setState(d.newState);
        break;

    case ActionType::StartMove:
    {
        Piece* piece = board_.getCell(d.from->row, d.from->col);
        int duration = calculateMoveTime(
            d.from->row, d.from->col, d.to->row, d.to->col);
        arbiter_.startMove(piece, *d.from, *d.to, duration);
        piece->setState(PieceState::Moving);
        stateMachine_.setSelectedCell(std::nullopt);
        stateMachine_.setState(d.newState);
        break;
    }

    case ActionType::StartJump:
    {
        Piece* piece = board_.getCell(d.from->row, d.from->col);
        arbiter_.startJump(piece, *d.from, JUMP_DURATION_MS);
        piece->setState(PieceState::Jumping);
        stateMachine_.setSelectedCell(std::nullopt);
        stateMachine_.setState(d.newState);
        break;
    }

    case ActionType::CancelSelection:
        stateMachine_.setSelectedCell(std::nullopt);
        stateMachine_.setState(d.newState);
        break;

    case ActionType::NoOp:
        break;
    }
}

void GameEngine::advanceTime(int milliseconds)
{
    arbiter_.advanceTime(milliseconds);

    // ── טיפול ב-Jumps שהושלמו ──
    auto completedJumps = arbiter_.pollCompletedJumps();
    for (const auto& jump : completedJumps)
    {
        if (jump.piece)
            jump.piece->setState(PieceState::Idle);
    }

    // ── טיפול ב-Moves שהושלמו ──
    auto completedMoves = arbiter_.pollCompletedMoves();

    // יירוטים: moves שבוטלו (wasCancelled=true) — קופץ יירט אויב
    for (const CompletedMove& cm : completedMoves)
    {
        if (cm.wasCancelled && cm.piece)
        {
            // האויב בוטל ע"י יירוט קפיצה — הקופץ לוכד אותו
            cm.piece->setState(PieceState::Captured);
            delete board_.takeCell(cm.from.row, cm.from.col);
            continue;
        }

        applyCompletedMove(cm);
        if (stateMachine_.getState() == GameState::GAME_OVER)
            return;
    }

    // ── חזרה למצב בחירה ──
    bool allDone = !arbiter_.hasActiveMoves() && !arbiter_.hasActiveJumps();
    if (allDone)
    {
        GameState s = stateMachine_.getState();
        if (s == GameState::MOVE_IN_PROGRESS || s == GameState::JUMP_IN_PROGRESS)
            stateMachine_.setState(GameState::WAITING_SELECTION);
    }
}

GameSnapshot GameEngine::getSnapshot() const
{
    GameSnapshot snapshot;
    snapshot.boardWidth = board_.colCount();
    snapshot.boardHeight = board_.rowCount();
    snapshot.selectedCell = stateMachine_.getSelectedCell();
    snapshot.gameOver = (stateMachine_.getState() == GameState::GAME_OVER);

    const auto& grid = board_.getGrid();
    for (int r = 0; r < board_.rowCount(); ++r)
    {
        for (int c = 0; c < board_.colCount(); ++c)
        {
            Piece* p = grid[r][c];
            if (!p)
                continue;

            PieceInfo info;
            info.kind = p->getType();
            info.color = p->getColor();
            info.pieceId = p->getId();
            info.cell = p->getCell();
            info.state = p->getState();
            info.progress = 0.0;

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
            else if (p->getState() == PieceState::Jumping)
            {
                for (const JumpEntry& jump : arbiter_.getActiveJumps())
                {
                    if (jump.piece == p)
                    {
                        info.progress = 1.0 -
                            static_cast<double>(jump.remainingMs) / JUMP_DURATION_MS;
                        break;
                    }
                }
            }

            snapshot.pieces.push_back(info);
        }
    }

    snapshot.winner = stateMachine_.getWinner();
    return snapshot;
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
        if (cm.piece)
            cm.piece->setState(PieceState::Idle);
        return;
    }

    // שליפת הכלי מהמקור — guard: ייתכן שכבר הוזז ע"י מהלך קודם
    Piece* piece = board_.getCell(cm.from.row, cm.from.col);
    if (piece != cm.piece)
        return;  // piece already moved (e.g. collision swap resolved by first mover)

    // בדיקת מלך לפני ביצוע המהלך
    Piece* targetPiece = board_.getCell(cm.to.row, cm.to.col);
    bool targetIsKing = targetPiece != nullptr &&
                        targetPiece->getType() == PieceType::King;

    // ביצוע המהלך דרך BoardController
    boardController_.executeMove(cm.piece, cm.from, cm.to);

    // טיפול בלכידת מלך
    if (targetIsKing)
    {
        stateMachine_.setState(GameState::GAME_OVER);
        if (cm.piece)
            stateMachine_.setWinner(cm.piece->getColor());
        return;
    }

    // הכתרה אוטומטית — חייל שהגיע לשורה האחרונה
    if (cm.piece && cm.piece->getType() == PieceType::Pawn)
    {
        int lastRow = (cm.piece->getColor() == Color::White) ? 0 : board_.rowCount() - 1;
        if (cm.to.row == lastRow)
            boardController_.promoteToQueen(cm.piece, cm.to);
    }
}
