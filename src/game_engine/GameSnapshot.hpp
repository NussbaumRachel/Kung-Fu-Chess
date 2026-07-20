#pragma once

#include "model/Constants.hpp"
#include "model/Position.hpp"
#include "model/Piece.hpp"
#include <vector>
#include <optional>

struct MoveRecord
{
    int minutes;
    int seconds;
    int milliseconds;
    PieceType pieceType;
    Color color;
    Position from;
    Position to;
    bool isJump;
    bool isCapture;
    bool givesCheck;  // reserved for future, always false for now
};
struct PieceInfo
{
    PieceType kind;
    Color color;
    int pieceId;
    Position cell;
    PieceState state;
    double progress;  // 0.0–1.0, רלוונטי רק ב-Moving
    std::optional<Position> targetCell;  // NEW: תא היעד בזמן Move
};


struct GameSnapshot
{
    int boardWidth = 0;
    int boardHeight = 0;
    std::vector<PieceInfo> pieces;
    std::optional<Position> selectedCell;
    bool gameOver = false;
    std::optional<Color> winner;
    int whiteScore = 0;
    int blackScore = 0;
    std::vector<MoveRecord> whiteMoves;
    std::vector<MoveRecord> blackMoves;
};




