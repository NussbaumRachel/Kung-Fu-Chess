#pragma once

#include "model/Constants.hpp"

class ScoreTracker
{
public:
    void awardPoints(Color scoringColor, int points);

    int whiteScore() const {
        return whiteScore_;
    }

    int blackScore() const {
        return blackScore_;
    }

private:
    int whiteScore_ = 0;
    int blackScore_ = 0;
};