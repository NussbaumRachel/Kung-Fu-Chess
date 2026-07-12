#pragma once

#include "Board.hpp"
#include "Piece.hpp"
#include "Position.hpp"
#include "types.hpp"

class BoardController
{
public:
    explicit BoardController(Board& board);

    // מבצע מהלך על הלוח:
    // - לוקח כלי מהמקור (takeCell)
    // - אם יש אויב ביעד — לוכד אותו (takeCell + delete)
    // - ממקם את הכלי ביעד (setCell)
    // - מעדכן את מצב הכלי ל-Idle
    // מחזיר מצביע לכלי שנלכד (nullptr אם לא היה כלי) — הקורא אחראי
    // לבדוק האם הכלי שנלכד הוא מלך לצורך סיום משחק.
    Piece* executeMove(Piece* piece, Position from, Position to);

    // בודק האם במיקום נתון נמצא כלי מסוג מלך
    bool isKingAt(Position pos) const;

    // מחליף חייל במלכה. מוחק את החייל, יוצר מלכה חדשה דרך PieceFactory.
    Piece* promoteToQueen(Piece* pawn, Position at);

private:
    Board& board_;
};
