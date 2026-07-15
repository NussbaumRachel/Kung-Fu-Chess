#pragma once

#include "model/Constants.hpp"
#include "model/Position.hpp"
#include "model/Piece.hpp"
#include <vector>
#include <optional>

struct PieceInfo
{
    PieceType kind;
    Color color;
    int pieceId;
    Position cell;
    PieceState state;
    double progress;  // 0.0–1.0, רלוונטי רק ב-Moving
};

struct GameSnapshot
{
    int boardWidth = 0;
    int boardHeight = 0;
    std::vector<PieceInfo> pieces;
    std::optional<Position> selectedCell;
    bool gameOver = false;
    std::optional<Color> winner;
};
