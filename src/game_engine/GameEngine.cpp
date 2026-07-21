#include "game_engine/GameEngine.hpp"
#include "model/Piece.hpp"
#include "movement/PieceFactory.hpp"
#include "game_engine/BoardController.hpp"
#include "game_engine/SnapshotBuilder.hpp"
#include <cstdlib>

GameEngine::GameEngine(Board board, const PieceSpeedConfig& speedConfig)
    : board_(std::move(board)),
      boardController_(board_),
      moveCompletionService_(boardController_, board_, stateMachine_),
      speedConfig_(speedConfig)
{
}

GameEngine::GameEngine(Board board)
    : GameEngine(std::move(board), PieceSpeedConfig{})
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

    bool pieceIsResting = false;
    if (!board_.isEmptyCell(row, col))
    {
        Piece* clickedPiece = board_.getCell(row, col);
        if (clickedPiece)
            pieceIsResting = cooldownService_.isResting(clickedPiece);
    }

    ClickContext ctx = clickPrepService_.prepare(
        board_, arbiter_, ruleEngine_,
        stateMachine_.getSelectedCell(),
        stateMachine_.getState(),
        row, col,
        pieceIsResting);

    GameState currentState = stateMachine_.getState();

    GameDecision d = stateMachine_.evaluate(
        ctx, currentState, stateMachine_.getSelectedCell());

    executeDecision(d);
}

void GameEngine::handleGameOver(std::optional<Color> winner)
{
    stateMachine_.setState(GameState::GAME_OVER);
    if (winner.has_value())
        stateMachine_.setWinner(winner.value());
}

void GameEngine::executeDecision(const GameDecision& d)
{
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
        int moveTime = calculateMoveTime(
            d.from->row, d.from->col, d.to->row, d.to->col, piece);
        arbiter_.startMove(piece, *d.from, *d.to, moveTime);
        piece->setState(PieceState::Moving);
        stateMachine_.setSelectedCell(std::nullopt);
        stateMachine_.setState(d.newState);
        break;
    }

    case ActionType::StartJump:
    {
        Piece* piece = board_.getCell(d.from->row, d.from->col);
        int jumpDuration = SnapshotBuilder::getJumpDurationMs(
            piece->getType(), piece->getColor(), speedConfig_);
        arbiter_.startJump(piece, Position{d.from->row, d.from->col},
            jumpDuration);
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

    if (board_.isEmptyCell(row, col) ||
        arbiter_.isPieceInvolved(row, col) ||
        stateMachine_.getState() == GameState::GAME_OVER)
        return;

    Piece* piece = board_.getCell(row, col);
    if (cooldownService_.isResting(piece))
        return;

    int jumpDuration = SnapshotBuilder::getJumpDurationMs(
        piece->getType(), piece->getColor(), speedConfig_);
    arbiter_.startJump(piece, Position{row, col}, jumpDuration);
    piece->setState(PieceState::Jumping);
    stateMachine_.setSelectedCell(std::nullopt);
    stateMachine_.setState(GameState::JUMP_IN_PROGRESS);
}

void GameEngine::advanceTime(int milliseconds)
{
    arbiter_.advanceTime(milliseconds, board_);
    gameTimeMs_ += milliseconds;
    moveRecorder_.setTimeMs(gameTimeMs_);

    // ── 1. טיפול בקפיצות שהושלמו ──
    for (const auto& jump : arbiter_.pollCompletedJumps())
    {
        if (jump.piece)
        {
            moveRecorder_.recordJump(jump);
            jump.piece->setState(PieceState::Short_rest);
            cooldownService_.startResting(jump.piece, CooldownService::REST_AFTER_JUMP_MS);
        }
    }

    // ── 2. טיפול במהלכים שהושלמו ──
    for (const CompletedMove& cm : arbiter_.pollCompletedMoves())
    {
        MoveCompletionResult result = moveCompletionService_.completeMove(cm);

        if (!cm.wasCancelled && cm.piece)
        {
            bool wasCapture = (result.pointsAwarded > 0);
            moveRecorder_.recordMove(cm, wasCapture);

            Piece* movedPiece = board_.getCell(cm.to.row, cm.to.col);
            if (movedPiece)
            {
                movedPiece->setState(PieceState::long_rest);
                cooldownService_.startResting(movedPiece, CooldownService::REST_AFTER_MOVE_MS);
            }
        }

        if (result.gameOver)
        {
            handleGameOver(result.winner);
            return;
        }

        if (result.pointsAwarded > 0)
            scoreTracker_.awardPoints(result.scoringColor, result.pointsAwarded);
    }

    // ── 3. קידום טיימרי מנוחה ──
    cooldownService_.advanceRestTimers(milliseconds);

    // ── 4. חזרה למצב בחירה ──
    maybeReturnToSelection();
}

void GameEngine::maybeReturnToSelection()
{
    if (!arbiter_.hasActiveMoves() && !arbiter_.hasActiveJumps())
    {
        GameState s = stateMachine_.getState();
        if (s == GameState::MOVE_IN_PROGRESS ||
            s == GameState::JUMP_IN_PROGRESS)
        {
            stateMachine_.setState(GameState::WAITING_SELECTION);
        }
    }
}

GameSnapshot GameEngine::getSnapshot() const
{
    SnapshotBuilder builder(board_, arbiter_, stateMachine_,
                            speedConfig_, scoreTracker_, moveRecorder_);
    return builder.build();
}

int GameEngine::calculateMoveTime(int fromRow, int fromCol,
                                  int toRow, int toCol, const Piece* piece) const
{
    int distance = std::max(std::abs(toRow - fromRow), std::abs(toCol - fromCol));
    if (!piece) return distance * 1000;
    double speed = speedConfig_.getMoveSpeed(piece->getType(), piece->getColor());
    if (speed <= 0.0) speed = 1.0;
    return static_cast<int>((distance / speed) * 1000.0);
}
