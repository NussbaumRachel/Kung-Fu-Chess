#pragma once

#include "game_engine/GameSnapshot.hpp"
#include "model/Board.hpp"
#include "model/Position.hpp"
#include "arbiter/RealTimeArbiter.hpp"
#include "game_engine/GameStateMachine.hpp"
#include "config/PieceSpeedConfig.hpp"
#include <unordered_map>

class Piece;
class ScoreTracker;
class MoveRecorder;

class SnapshotBuilder
{
public:
    SnapshotBuilder(const Board& board,
                    const RealTimeArbiter& arbiter,
                    const GameStateMachine& stateMachine,
                    const PieceSpeedConfig& speedConfig,
                    const ScoreTracker& scoreTracker,
                    const MoveRecorder& moveRecorder);

    GameSnapshot build() const;
    static int getJumpDurationMs(PieceType type,
                             Color color,
                             const PieceSpeedConfig& speedConfig);
private:
    const Board& board_;
    const RealTimeArbiter& arbiter_;
    const GameStateMachine& stateMachine_;
    const PieceSpeedConfig& speedConfig_;
    const ScoreTracker& scoreTracker_;
    const MoveRecorder& moveRecorder_;

    std::unordered_map<const Piece*, double> buildMoveProgressMap() const;
    std::unordered_map<const Piece*, Position> buildMoveTargetMap() const;
    std::unordered_map<const Piece*, double> buildJumpProgressMap() const;
    int getJumpDurationMs(PieceType type, Color color) const;
};
