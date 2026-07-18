#include "game_engine/GameEngine.hpp"
#include "model/Piece.hpp"
#include "movement/PieceFactory.hpp"
#include "game_engine/BoardController.hpp"
#include <cstdlib>

GameEngine::GameEngine(Board board)
    : board_(std::move(board)),
      boardController_(board_),
      moveCompletionService_(boardController_, board_, stateMachine_)
      // clickPrepService_ מאותחל אוטומטית (default constructor)
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

    ClickContext ctx = clickPrepService_.prepare(
        board_, arbiter_, ruleEngine_,
        stateMachine_.getSelectedCell(),
        stateMachine_.getState(),
        row, col);

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
            d.from->row, d.from->col, d.to->row, d.to->col);
        arbiter_.startMove(piece, *d.from, *d.to, moveTime);
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

    // אפשר לקפוץ רק כשיש כלי פנוי והמשחק לא נגמר
    if (board_.isEmptyCell(row, col) ||
        arbiter_.isPieceInvolved(row, col) ||
        stateMachine_.getState() == GameState::GAME_OVER)
        return;

    Piece* piece = board_.getCell(row, col);
    arbiter_.startJump(piece, Position{row, col}, JUMP_DURATION_MS);
    piece->setState(PieceState::Jumping);
    stateMachine_.setSelectedCell(std::nullopt);
    stateMachine_.setState(GameState::JUMP_IN_PROGRESS);
}

void GameEngine::advanceTime(int milliseconds)
{
    arbiter_.advanceTime(milliseconds);

    // טיפול בקפיצות שהושלמו
    for (const auto& jump : arbiter_.pollCompletedJumps())
    {
        if (jump.piece)
            jump.piece->setState(PieceState::Idle);
    }

    // טיפול במהלכים שהושלמו
    for (const CompletedMove& cm : arbiter_.pollCompletedMoves())
    {
        MoveCompletionResult result = moveCompletionService_.completeMove(cm);
        if (result.gameOver)
        {
            handleGameOver(result.winner);
            return;   // המשחק נגמר — לא ממשיכים
        }
    }

    // חזרה למצב בחירה אם הכל שקט
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
    // Pre-compute maps — O(activeMoves + activeJumps) במקום O(pieces × ...)
    auto moveProgressMap = buildMoveProgressMap();
    auto moveTargetMap   = buildMoveTargetMap();
    auto jumpProgressMap = buildJumpProgressMap();

    GameSnapshot snapshot;
    snapshot.boardWidth  = board_.colCount();
    snapshot.boardHeight = board_.rowCount();
    snapshot.selectedCell = stateMachine_.getSelectedCell();
    snapshot.gameOver    = (stateMachine_.getState() == GameState::GAME_OVER);

    const auto& grid = board_.getGrid();
    for (int r = 0; r < board_.rowCount(); ++r)
    {
        for (int c = 0; c < board_.colCount(); ++c)
        {
            const Piece* p = grid[r][c].get();
            if (!p) continue;

            PieceInfo info;
            info.kind    = p->getType();
            info.color   = p->getColor();
            info.pieceId = p->getId();
            info.cell    = p->getCell();
            info.state   = p->getState();
            info.progress = 0.0;

            if (p->getState() == PieceState::Moving)
            {
                auto it = moveProgressMap.find(p);
                if (it != moveProgressMap.end())
                {
                    info.progress   = it->second;
                    info.targetCell = moveTargetMap[p];
                }
            }
            else if (p->getState() == PieceState::Jumping)
            {
                auto it = jumpProgressMap.find(p);
                if (it != jumpProgressMap.end())
                    info.progress = it->second;
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
std::unordered_map<const Piece*, double> GameEngine::buildMoveProgressMap() const
{
    std::unordered_map<const Piece*, double> map;
    for (const Move& move : arbiter_.getActiveMoves())
    {
        if (move.getPiece())
            map[move.getPiece()] = move.getProgress();
    }
    return map;
}
std::unordered_map<const Piece*, Position> GameEngine::buildMoveTargetMap() const
{
    std::unordered_map<const Piece*, Position> map;
    for (const Move& move : arbiter_.getActiveMoves())
    {
        if (move.getPiece())
            map[move.getPiece()] = move.getTo();
    }
    return map;
}
std::unordered_map<const Piece*, double> GameEngine::buildJumpProgressMap() const
{
    std::unordered_map<const Piece*, double> map;
    for (const JumpEntry& jump : arbiter_.getActiveJumps())
    {
        if (jump.piece)
        {
            map[jump.piece] = 1.0 -
                static_cast<double>(jump.remainingMs) / JUMP_DURATION_MS;
        }
    }
    return map;
}
void GameEngine::startResting(Piece* piece, int durationMs)
{
    if (piece)
    {
        piece->setState(PieceState::Resting);
        restTimers_[piece] = durationMs;
    }
}
void GameEngine::advanceRestTimers(int milliseconds)
{
    for (auto it = restTimers_.begin(); it != restTimers_.end(); )
    {
        it->second -= milliseconds;
        if (it->second <= 0)
        {
            // הכלי סיים מנוחה
            if (it->first)
                it->first->setState(PieceState::Idle);
            it = restTimers_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
bool GameEngine::isResting(const Piece* piece) const
{
    return restTimers_.find(piece) != restTimers_.end();
}
