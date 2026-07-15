#pragma once

struct Position
{
    int row;
    int col;

    bool operator==(const Position& other) const
    {
        return row == other.row &&
               col == other.col;
    }
};