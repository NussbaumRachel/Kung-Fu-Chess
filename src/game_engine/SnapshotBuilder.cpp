#include "game_engine/SnapshotBuilder.hpp"
#include "game_engine/ScoreTracker.hpp"
#include "game_engine/MoveRecorder.hpp"
#include "model/Piece.hpp"

SnapshotBuilder::SnapshotBuilder(const Board& board,
                                 const RealTimeArbiter& arbiter,
                                 const GameStateMachine& stateMachine,
                                 const PieceSpeedConfig& speedConfig,
                                 const ScoreTracker& scoreTracker,
                                 const MoveRecorder& moveRecorder)
    : board_(board),
      arbiter_(arbiter),
      stateMachine_(stateMachine),
      speedConfig_(speedConfig),
      scoreTracker_(scoreTracker),
      moveRecorder_(moveRecorder)
{
}

GameSnapshot SnapshotBuilder::build() const
{
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
    snapshot.whiteScore = scoreTracker_.whiteScore();
    snapshot.blackScore = scoreTracker_.blackScore();
    snapshot.whiteMoves = moveRecorder_.whiteMoves();
    snapshot.blackMoves = moveRecorder_.blackMoves();

    return snapshot;
}

std::unordered_map<const Piece*, double> SnapshotBuilder::buildMoveProgressMap() const
{
    std::unordered_map<const Piece*, double> map;
    for (const Move& move : arbiter_.getActiveMoves())
    {
        if (move.getPiece())
            map[move.getPiece()] = move.getProgress();
    }
    return map;
}

std::unordered_map<const Piece*, Position> SnapshotBuilder::buildMoveTargetMap() const
{
    std::unordered_map<const Piece*, Position> map;
    for (const Move& move : arbiter_.getActiveMoves())
    {
        if (move.getPiece())
            map[move.getPiece()] = move.getTo();
    }
    return map;
}

std::unordered_map<const Piece*, double> SnapshotBuilder::buildJumpProgressMap() const
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

int SnapshotBuilder::getJumpDurationMs(PieceType type, Color color) const
{
    return SnapshotBuilder::getJumpDurationMs(type, color, speedConfig_);
}

int SnapshotBuilder::getJumpDurationMs(PieceType type, Color color,
                                        const PieceSpeedConfig& speedConfig)
{
    double speed = speedConfig.getJumpSpeed(type, color);
    if (speed <= 0.0) speed = 3.0;
    return static_cast<int>((1.0 / speed) * 1000.0);
}
