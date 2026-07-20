#include "game_engine/GameEngine.hpp"
#include "model/Piece.hpp"
#include "movement/PieceFactory.hpp"
#include "game_engine/BoardController.hpp"
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

            // בדיקה: האם התא מכיל כלי במנוחה?
    bool pieceIsResting = false;
    if (!board_.isEmptyCell(row, col))
    {
        Piece* clickedPiece = board_.getCell(row, col);
        if (clickedPiece)
            pieceIsResting = isResting(clickedPiece);
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
        arbiter_.startJump(piece, Position{d.from->row,d.from-> col},
        getJumpDurationMs(piece->getType(), piece->getColor()));
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
    if (isResting(piece))
        return;
    arbiter_.startJump(piece, Position{row, col},
        getJumpDurationMs(piece->getType(), piece->getColor()));
    piece->setState(PieceState::Jumping);
    stateMachine_.setSelectedCell(std::nullopt);
    stateMachine_.setState(GameState::JUMP_IN_PROGRESS);
}

void GameEngine::advanceTime(int milliseconds)
{
    arbiter_.advanceTime(milliseconds, board_);
    gameTimeMs_ += milliseconds;

    // ── 1. טיפול בקפיצות שהושלמו ──
    for (const auto& jump : arbiter_.pollCompletedJumps())
    {
        if (jump.piece)
        {
            recordJump(jump);
            jump.piece->setState(PieceState::Short_rest);
            startResting(jump.piece, REST_AFTER_JUMP_MS);  // 500ms מנוחה
        }
    }
    // ── 2. טיפול במהלכים שהושלמו ──
        for (const CompletedMove& cm : arbiter_.pollCompletedMoves())
    {
        MoveCompletionResult result = moveCompletionService_.completeMove(cm);
        if (!cm.wasCancelled && cm.piece)
        {
            bool wasCapture = (result.pointsAwarded > 0);
            recordMove(cm, wasCapture);
        }

        if (!cm.wasCancelled && cm.piece)
        {
            // המהלך הושלם בהצלחה — הכלי עכשיו ב-to
            Piece* movedPiece = board_.getCell(cm.to.row, cm.to.col);
            if (movedPiece)
            {
                movedPiece->setState(PieceState::long_rest);      // ← נקי: קודם Idle
                startResting(movedPiece, REST_AFTER_MOVE_MS); // ← ואז Resting
            }
        }

        if (result.gameOver)
        {
            handleGameOver(result.winner);
            return;
        }
        if (result.pointsAwarded > 0) {
            if (result.scoringColor == Color::White)
                whiteScore_ += result.pointsAwarded;
            else
                blackScore_ += result.pointsAwarded;
        }
    }



    // ── 3. קידום טיימרי מנוחה ──
    advanceRestTimers(milliseconds);

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
    snapshot.whiteScore = whiteScore_;
    snapshot.blackScore = blackScore_;
    snapshot.whiteMoves = whiteMoves_;
    snapshot.blackMoves = blackMoves_;

    return snapshot;
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
                       int dur = getJumpDurationMs(jump.piece->getType(), jump.piece->getColor());
            map[jump.piece] = 1.0 -
                (dur > 0 ? static_cast<double>(jump.remainingMs) / dur : 0.0);

        }
    }
    return map;
}
int GameEngine::getJumpDurationMs(PieceType type, Color color) const
{
    double speed = speedConfig_.getJumpSpeed(type, color);
    if (speed <= 0.0) speed = 3.0;
    // קפיצה היא תמיד תא בודד (distance=1)
    return static_cast<int>((1.0 / speed) * 1000.0);
}

void GameEngine::startResting(Piece* piece, int durationMs)
{
    if (piece)
    {
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
bool GameEngine::isResting(Piece* piece) const
{
    return restTimers_.find(piece) != restTimers_.end();
}
void GameEngine::recordMove(const CompletedMove& cm, bool isCapture)
{
    if (!cm.piece) return;

    int totalMs = gameTimeMs_;
    int mins = totalMs / 60000;
    int secs = (totalMs / 1000) % 60;
    int ms   = totalMs % 1000;

    MoveRecord mr;
    mr.minutes      = mins;
    mr.seconds      = secs;
    mr.milliseconds = ms;
    mr.pieceType    = cm.piece->getType();
    mr.color        = cm.piece->getColor();
    mr.from         = cm.from;
    mr.to           = cm.to;
    mr.isJump       = false;
    mr.isCapture    = isCapture;
    mr.givesCheck   = false;

    if (mr.color == Color::White)
        whiteMoves_.push_back(mr);
    else
        blackMoves_.push_back(mr);
}

void GameEngine::recordJump(const JumpEntry& jump)
{
    if (!jump.piece) return;

    int totalMs = gameTimeMs_;
    int mins = totalMs / 60000;
    int secs = (totalMs / 1000) % 60;
    int ms   = totalMs % 1000;

    MoveRecord mr;
    mr.minutes      = mins;
    mr.seconds      = secs;
    mr.milliseconds = ms;
    mr.pieceType    = jump.piece->getType();
    mr.color        = jump.piece->getColor();
    mr.from         = jump.piece->getCell();  // jump is in-place, from=to
    mr.to           = jump.cell;
    mr.isJump       = true;
    mr.isCapture    = false;
    mr.givesCheck   = false;

    if (mr.color == Color::White)
        whiteMoves_.push_back(mr);
    else
        blackMoves_.push_back(mr);
}
