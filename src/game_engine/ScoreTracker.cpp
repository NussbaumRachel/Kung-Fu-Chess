#include "game_engine/ScoreTracker.hpp"

void ScoreTracker::awardPoints(
    Color scoringColor,
    int points)
{
    if (scoringColor == Color::White)
        whiteScore_ += points;
    else
        blackScore_ += points;
}