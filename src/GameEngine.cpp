#include "GameEngine.hpp"
#include "Piece.hpp"
#include "PieceFactory.hpp"
#include "BoardController.hpp"
#include <cstdlib>

GameEngine::GameEngine(Board board)
    :
    board_(std::move(board)),
    boardController_(board_),
    moveCompletionService_( boardController_, board_, stateMachine_)
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

void GameEngine::handleJump(int row, int col)
{
    if (!board_.isInsideBoard(row, col))
        return;

    // אפשר לקפוץ רק כשיש כלי, הוא לא בתנועה/קפיצה, והמשחק לא נגמר
    if (board_.isEmptyCell(row, col))
        return;

    if (arbiter_.isPieceInvolved(row, col))
        return;

    if (stateMachine_.getState() == GameState::GAME_OVER)
        return;

    Piece* piece = board_.getCell(row, col);
    Position pos{row, col};
    arbiter_.startJump(piece, pos, JUMP_DURATION_MS);
    piece->setState(PieceState::Jumping);
    stateMachine_.setSelectedCell(std::nullopt);
    stateMachine_.setState(GameState::JUMP_IN_PROGRESS);
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
        MoveCompletionResult result =
            moveCompletionService_.completeMove(cm);
        
        if (result.gameOver)
        {
            stateMachine_.setState(GameState::GAME_OVER);

            if (result.winner.has_value())
                stateMachine_.setWinner(result.winner.value());

            return;
        }
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
            Piece* p = grid[r][c].get();
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
    int distance = std::max(std::abs(toRow - fromRow), std::abs(toCol - fromCol));
    return distance * 1000;
}

// void GameEngine::applyCompletedMove(const CompletedMove& completedMove)
// {
//     MoveCompletionResult result =
//         moveCompletionService_.completeMove(completedMove);

//     if (!result.gameOver)
//         return;

//     stateMachine_.setState(GameState::GAME_OVER);

//     if (result.winner.has_value())
//         stateMachine_.setWinner(result.winner.value());
// }