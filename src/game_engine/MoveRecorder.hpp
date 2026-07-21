#pragma once

#include "game_engine/GameSnapshot.hpp"
#include "movement/CompletedMove.hpp"
#include "model/Constants.hpp"
#include <vector>

struct JumpEntry;
class Piece;

class MoveRecorder
{
public:
    void setTimeMs(int gameTimeMs) { gameTimeMs_ = gameTimeMs; }

    void recordMove(const CompletedMove& cm, bool isCapture);
    void recordJump(const JumpEntry& jump);

    const std::vector<MoveRecord>& whiteMoves() const { return whiteMoves_; }
    const std::vector<MoveRecord>& blackMoves() const { return blackMoves_; }

private:
    int gameTimeMs_ = 0;
    std::vector<MoveRecord> whiteMoves_;
    std::vector<MoveRecord> blackMoves_;
};
