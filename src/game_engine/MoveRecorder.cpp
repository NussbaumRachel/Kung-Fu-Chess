#include "game_engine/MoveRecorder.hpp"
#include "model/Piece.hpp"
#include "arbiter/RealTimeArbiter.hpp"  

void MoveRecorder::recordMove(const CompletedMove& cm, bool isCapture)
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

void MoveRecorder::recordJump(const JumpEntry& jump)
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
